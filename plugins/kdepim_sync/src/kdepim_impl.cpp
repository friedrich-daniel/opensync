/*********************************************************************** 
Actual implementation of the KDE PIM OpenSync plugin
Copyright (C) 2004 Conectiva S. A.
Based on code Copyright (C) 2004 Stewart Heitmann <sheitmann@users.sourceforge.net>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation;

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.
IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) AND AUTHOR(S) BE LIABLE FOR ANY
CLAIM, OR ANY SPECIAL INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES 
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN 
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF 
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

ALL LIABILITY, INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PATENTS, 
COPYRIGHTS, TRADEMARKS OR OTHER RIGHTS, RELATING TO USE OF THIS 
SOFTWARE IS DISCLAIMED.
*************************************************************************/
/**
 * @autor Eduardo Pereira Habkost <ehabkost@conectiva.com.br>
 */

extern "C"
{
#include <opensync/opensync.h>
}

#include <kabc/stdaddressbook.h>
#include <kabc/vcardconverter.h>
#include <kabc/resource.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <klocale.h>
#include <qsignal.h>
#include <qfile.h> 

#include "osyncbase.h"
#include "kaddrbook.h"


static
void unfold_vcard(char *vcard, size_t *size)
{
    char* in  = vcard;
    char* out = vcard;
    char *end = vcard + *size;
    while ( in < end)
    {
        /* remove any occurrences of "=[CR][LF]"                */
        /* these denote folded line markers in VCARD format.    */
        /* Dont know why, but Evolution uses the leading "="    */
        /* character to (presumably) denote a control sequence. */
        /* This is not quite how I interpret the VCARD RFC2426  */
        /* spec (section 2.6 line delimiting and folding).      */
        /* This seems to work though, so thats the main thing!  */
        if (in[0]=='=' && in[1]==13 && in[2]==10)
            in+=3;
        else
            *out++ = *in++;
    }
    *size = out - vcard;
}

static KApplication *applicationptr=NULL;
static char name[] = "kde-opensync-plugin";
static char *argv[] = {name,0};

class KdePluginImplementation: public KdePluginImplementationBase
{
    private:
        KABC::AddressBook* addressbookptr;   
        KABC::Ticket* addressbookticket;
        QDateTime syncdate, newsyncdate;

        OSyncMember *member;
        OSyncHashTable *hashtable;

    public:
        KdePluginImplementation(OSyncMember *memb)
            :member(memb)
        {
            //osync_debug("kde", 3, "%s(%s)", __FUNCTION__);

            KCmdLineArgs::init(1, argv, "kde-opensync-plugin", i18n("KOpenSync"), "KDE OpenSync plugin", "0.1", false);
            applicationptr = new KApplication();

            //get a handle to the standard KDE addressbook
            addressbookptr = KABC::StdAddressBook::self();

            //ensure a NULL Ticket ptr
            addressbookticket=NULL;

            hashtable = osync_hashtable_new();
            osync_hashtable_load(hashtable, member);

        }
        
        virtual ~KdePluginImplementation()
        {
            if (applicationptr) {
                delete applicationptr;
                applicationptr = NULL;
            }
        }

        /** Calculate the hash value for an Addressee.
         * Should be called before returning/writing the
         * data, because the revision of the Addressee
         * can be changed.
         */
        QString calc_hash(KABC::Addressee &e)
        {
            //Get the revision date of the KDE addressbook entry.
            //Regard entries with invalid revision dates as having just been changed.
            QDateTime revdate = e.revision();
            if (!revdate.isValid())
            {
                revdate = newsyncdate;      //use date of this sync as the revision date.
                e.setRevision(revdate);   //update the Addressbook entry for future reference.
            }
            return revdate.toString();
        }

        virtual void connect(OSyncContext *ctx)
        {
            //Lock the addressbook
            addressbookticket = addressbookptr->requestSaveTicket();

            if (!addressbookticket)
            {
                osync_context_report_error(ctx, OSYNC_ERROR_GENERIC, "Couldn't lock KDE addressbook");
                return;
            }
            osync_debug("kde", 3, "KDE addressbook locked OK.");

            osync_context_report_success(ctx);
        }

        virtual void disconnect(OSyncContext *ctx)
        {
            //Unlock the addressbook
            addressbookptr->save(addressbookticket);
            addressbookticket = NULL;

            osync_context_report_success(ctx);
        }


        virtual void get_changes(OSyncContext *ctx)
        {
            //osync_debug("kde", 3, "kaddrbook::%s(newdbs=%d)", __FUNCTION__, newdbs);

            //FIXME: should I detect if slow_sync is necessary?
            osync_bool slow_sync = osync_member_get_slow_sync(member, "contact");

            //remember when we started this current sync
            newsyncdate = QDateTime::currentDateTime();

            // We must reload the KDE addressbook in order to retrieve the latest changes.
            if (!addressbookptr->load())
            {
                osync_context_report_error(ctx, OSYNC_ERROR_GENERIC, "Couldn't reload KDE addressbook");
                return;
            }
            osync_debug("kde", 3, "KDE addressbook reloaded OK.");

            //osync_debug("kde", 3, "%s: %s : plugin UID list has %d entries", __FILE__, __FUNCTION__, uidlist.count());

            for (KABC::AddressBook::Iterator it=addressbookptr->begin(); it!=addressbookptr->end(); it++ ) {
                osync_debug("kde", 3, "new entry, uid: %s", it->uid().latin1());

                QString hash = calc_hash(*it);

                // gmalloc a changed_object for this phonebook entry             
                //FIXME: deallocate it somewhere
                OSyncChange *chg= osync_change_new();
                osync_change_set_member(chg, member);

                osync_change_set_hash(chg, hash);
                osync_change_set_uid(chg, it->uid().latin1());

                // Convert the VCARD data into a string
                KABC::VCardConverter converter;
                QString card = converter.createVCard(*it);
                QString data(card.latin1());
                //FIXME: deallocate data somewhere
                osync_change_set_data(chg, strdup(data), data.length(), 1);

                // object type and format
                osync_change_set_objtype_string(chg, "contact");
                osync_change_set_objformat_string(chg, "vcard");

                // Use the hash table to check if the object
                // needs to be reported
                osync_change_set_hash(chg, hash.data());
                if (osync_hashtable_detect_change(hashtable, chg, slow_sync)) {
                    osync_context_report_change(ctx, chg);
                    osync_hashtable_update_hash(hashtable, chg);
                }
            }

            // Use the hashtable to report deletions
            osync_hashtable_report_deleted(hashtable, ctx, slow_sync);

            osync_context_report_success(ctx);
        }


        /** Access an object, without returning success
         *
         * returns 0 on success, < 0 on error.
         * If an error occurss, the error will be already reported
         * using osync_context_report_error()
         */
        int __vcard_access(OSyncContext *ctx, OSyncChange *chg)
        {
            //osync_debug("kde", 3, "kaddrbook::%s()",__FUNCTION__);

            // Ensure we still have a lock on the KDE addressbook (we ought to)
            if (addressbookticket==NULL)
            {
                //This should never happen, but just in case....
                osync_context_report_error(ctx, OSYNC_ERROR_GENERIC, "Lock on KDE addressbook was lost");
                return -1;
            }

            KABC::VCardConverter converter;
    
            OSyncChangeType chtype = osync_change_get_changetype(chg);
            char *uid = osync_change_get_uid(chg);
            /* treat modified objects without UIDs as if they were newly added objects */
            if (chtype == CHANGE_MODIFIED && !uid)
                chtype = CHANGE_ADDED;

            // convert VCARD string from obj->comp into an Addresse object.
            char *data;
            size_t data_size;
            data = (char*)osync_change_get_data(chg);
            data_size = osync_change_get_datasize(chg);

            switch(chtype)
            {
                case CHANGE_MODIFIED:
                {
                    unfold_vcard(data, &data_size);

                    KABC::Addressee addressee = converter.parseVCard(QString::fromLatin1(data, data_size));

                    // ensure it has the correct UID
                    addressee.setUid(QString(uid));
                    QString hash = calc_hash(addressee);

                    // replace the current addressbook entry (if any) with the new one
                    addressbookptr->insertAddressee(addressee);
                    osync_change_set_hash(chg, hash);
                    osync_debug("kde", 3, "KDE ADDRESSBOOK ENTRY UPDATED (UID=%s)", uid); 
                    break;
                }

                case CHANGE_ADDED:
                {
                    // convert VCARD string from obj->comp into an Addresse object
                    // KABC::VCardConverter doesnt do VCARD unfolding so we must do it ourselves first.
                    unfold_vcard(data, &data_size);
                    KABC::Addressee addressee = converter.parseVCard(QString::fromLatin1(data, data_size));

                    QString hash = calc_hash(addressee);

                    // add the new address to the addressbook
                    addressbookptr->insertAddressee(addressee);

                    // return the UID of the new entry along with the result
                    osync_change_set_uid(chg, addressee.uid().latin1());
                    osync_change_set_hash(chg, hash);
                    osync_debug("kde", 3, "KDE ADDRESSBOOK ENTRY ADDED (UID=%s)", addressee.uid().latin1());

                    break;
                }

                case CHANGE_DELETED:
                {
                    if (uid==NULL)
                    {
                        osync_context_report_error(ctx, OSYNC_ERROR_FILE_NOT_FOUND, "Entry with null UID not found");
                        return -1;
                    }

                    //find addressbook entry with matching UID and delete it
                    KABC::Addressee addressee = addressbookptr->findByUid(QString(uid));
                    if(!addressee.isEmpty())
                       addressbookptr->removeAddressee(addressee);

                    osync_debug("kde", 3, "KDE ADDRESSBOOK ENTRY DELETED (UID=%s)", uid);

                    break;
                }
                default:
                    osync_context_report_error(ctx, OSYNC_ERROR_NOT_SUPPORTED, "Operation not supported");
                    return -1;
            }
            //Save the changes without dropping the lock
            addressbookticket->resource()->save(addressbookticket);
    
            return 0;
        }

        virtual bool vcard_access(OSyncContext *ctx, OSyncChange *chg)
        {
            if (__vcard_access(ctx, chg) < 0)
                return false;
            osync_context_report_success(ctx);
            /*FIXME: What should be returned? */
            return true;
        }

        virtual bool vcard_commit_change(OSyncContext *ctx, OSyncChange *chg)
        {
            if ( __vcard_access(ctx, chg) < 0)
                return false;
            osync_hashtable_update_hash(hashtable, chg);
            osync_context_report_success(ctx);
            /*FIXME: What should be returned? */
            return true;
        }

};


extern "C" {

KdePluginImplementationBase *new_implementation_object(OSyncMember *member)
{
    return new KdePluginImplementation(member);
}

}// extern "C"