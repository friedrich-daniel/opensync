#include "support.h"

static void conv_vcard(const char *filename, const char *extension)
{
	OSyncError *error = NULL;
	OSyncEnv *env = init_env();
	
	OSyncFormatEnv *conv_env = osync_conv_env_new(env);
	fail_unless(conv_env != NULL, NULL);

	char *buffer;
	int size;
	
	fail_unless(osync_file_read(filename, &buffer, &size, &error), NULL);
	
	OSyncChange *change = osync_change_new();
	osync_change_set_uid(change, filename);		
	osync_change_set_data(change, buffer, size + 1, TRUE);
	osync_change_set_conv_env(change, conv_env);
	
	osync_change_set_objformat_string(change, "plain");

	OSyncObjFormat *sourceformat = osync_change_detect_objformat(conv_env, change, &error);
	fail_unless(sourceformat != NULL, NULL);
	osync_change_set_objformat(change, sourceformat);
	osync_change_set_objtype(change, osync_objformat_get_objtype(sourceformat));
	
	OSyncObjFormat *targetformat = NULL;
	if (!strcmp(osync_objformat_get_name(sourceformat), "vcard21"))
		targetformat = osync_conv_find_objformat(conv_env, "vcard30");
	
	if (!strcmp(osync_objformat_get_name(sourceformat), "vcard30"))
		targetformat = osync_conv_find_objformat(conv_env, "vcard21");

	fail_unless(targetformat != NULL, NULL);
	
	OSyncChange *newchange = osync_change_copy(change, &error);
	fail_unless(newchange != NULL, NULL);
	
	//Convert to
	fail_unless(osync_change_convert_extension(conv_env, change, targetformat, extension, &error), NULL);
	
	//Compare old to new
	fail_unless(osync_change_compare(newchange, change) == CONV_DATA_SAME, NULL);
	
	//Convert back
	fail_unless(osync_change_convert_extension(conv_env, change, targetformat, extension, &error), NULL);
	
	//Compare again
	fail_unless(osync_change_compare(newchange, change) == CONV_DATA_SAME, NULL);
	
	osync_conv_env_free(conv_env);
	osync_env_finalize(env, NULL);
	osync_env_free(env);
}

static void compare_vcard(const char *lfile, const char *rfile, OSyncConvCmpResult result)
{
	OSyncError *error = NULL;
	OSyncEnv *env = init_env();
	
	OSyncFormatEnv *conv_env = osync_conv_env_new(env);
	fail_unless(conv_env != NULL, NULL);

	char *buffer;
	int size;
	
	fail_unless(osync_file_read(lfile, &buffer, &size, &error), NULL);
	
	OSyncChange *lchange = osync_change_new();
	osync_change_set_uid(lchange, lfile);		
	osync_change_set_data(lchange, buffer, size + 1, TRUE);
	osync_change_set_conv_env(lchange, conv_env);
	osync_change_set_objformat_string(lchange, "plain");

	OSyncObjFormat *sourceformat = osync_change_detect_objformat(conv_env, lchange, &error);
	fail_unless(sourceformat != NULL, NULL);
	osync_change_set_objformat(lchange, sourceformat);
	osync_change_set_objtype(lchange, osync_objformat_get_objtype(sourceformat));
	
	
	fail_unless(osync_file_read(rfile, &buffer, &size, &error), NULL);
	
	OSyncChange *rchange = osync_change_new();
	osync_change_set_uid(rchange, lfile);		
	osync_change_set_data(rchange, buffer, size + 1, TRUE);
	osync_change_set_conv_env(rchange, conv_env);
	osync_change_set_objformat_string(rchange, "plain");

	sourceformat = osync_change_detect_objformat(conv_env, rchange, &error);
	fail_unless(sourceformat != NULL, NULL);
	osync_change_set_objformat(rchange, sourceformat);
	osync_change_set_objtype(rchange, osync_objformat_get_objtype(sourceformat));
	
	fail_unless(osync_change_compare(lchange, rchange) == result, NULL);
	
	osync_conv_env_free(conv_env);
	osync_env_finalize(env, NULL);
	osync_env_free(env);
}

START_TEST (conv_vcard_evolution2_full1)
{
	conv_vcard("data/vcards/evolution2/evo2-full1.vcf", "evolution");
}
END_TEST

START_TEST (conv_vcard_evolution2_full2)
{
	conv_vcard("data/vcards/evolution2/evo2-full2.vcf", "evolution");
}
END_TEST

START_TEST (conv_vcard_evolution2_photo)
{
	conv_vcard("data/vcards/evolution2/evo2-photo.vcf", "evolution");
}
END_TEST

START_TEST (conv_vcard_evolution2_multiline)
{
	conv_vcard("data/vcards/evolution2/evo2-multiline.vcf", "evolution");
}
END_TEST

START_TEST (conv_vcard_evolution2_umlaute)
{
	conv_vcard("data/vcards/evolution2/evo2-umlaute.vcf", "evolution");
}
END_TEST

START_TEST (conv_vcard_evolution2_special)
{
	conv_vcard("data/vcards/evolution2/evo2-special.vcf", "evolution");
}
END_TEST

START_TEST (conv_vcard_kde_21_full1)
{
	conv_vcard("data/vcards/kdepim/kdepim-full1-2.1.vcf", "kde");
}
END_TEST

START_TEST (conv_vcard_kde_30_full1)
{
	conv_vcard("data/vcards/kdepim/kdepim-full1-3.0.vcf", "kde");
}
END_TEST

START_TEST (conv_vcard_kde_21_full2)
{
	conv_vcard("data/vcards/kdepim/kdepim-full2-2.1.vcf", "kde");
}
END_TEST

START_TEST (conv_vcard_kde_30_full2)
{
	conv_vcard("data/vcards/kdepim/kdepim-full2-3.0.vcf", "kde");
}
END_TEST

START_TEST (conv_vcard_kde_21_multiline)
{
	conv_vcard("data/vcards/kdepim/kdepim-multiline-2.1.vcf", "kde");
}
END_TEST

START_TEST (conv_vcard_kde_30_multiline)
{
	conv_vcard("data/vcards/kdepim/kdepim-multiline-3.0.vcf", "kde");
}
END_TEST

START_TEST (conv_vcard_kde_21_photo1)
{
	conv_vcard("data/vcards/kdepim/kdepim-photo1-2.1.vcf", "kde");
}
END_TEST

START_TEST (conv_vcard_kde_30_photo1)
{
	conv_vcard("data/vcards/kdepim/kdepim-photo1-3.0.vcf", "kde");
}
END_TEST

START_TEST (conv_vcard_kde_21_photo2)
{
	conv_vcard("data/vcards/kdepim/kdepim-photo2-2.1.vcf", "kde");
}
END_TEST

START_TEST (conv_vcard_kde_30_photo2)
{
	conv_vcard("data/vcards/kdepim/kdepim-photo2-3.0.vcf", "kde");
}
END_TEST

START_TEST (conv_vcard_kde_21_sound1)
{
	conv_vcard("data/vcards/kdepim/kdepim-sound1-2.1.vcf", "kde");
}
END_TEST

START_TEST (conv_vcard_kde_30_sound1)
{
	conv_vcard("data/vcards/kdepim/kdepim-sound1-3.0.vcf", "kde");
}
END_TEST

START_TEST (conv_vcard_kde_21_sound2)
{
	conv_vcard("data/vcards/kdepim/kdepim-sound2-2.1.vcf", "kde");
}
END_TEST

START_TEST (conv_vcard_kde_30_sound2)
{
	conv_vcard("data/vcards/kdepim/kdepim-sound2-3.0.vcf", "kde");
}
END_TEST

START_TEST (conv_vcard_kde_21_special)
{
	conv_vcard("data/vcards/kdepim/kdepim-special-2.1.vcf", "kde");
}
END_TEST

START_TEST (conv_vcard_kde_30_special)
{
	conv_vcard("data/vcards/kdepim/kdepim-special-3.0.vcf", "kde");
}
END_TEST

START_TEST (conv_vcard_kde_21_umlaute)
{
	conv_vcard("data/vcards/kdepim/kdepim-umlaute-2.1.vcf", "kde");
}
END_TEST

START_TEST (conv_vcard_kde_30_umlaute)
{
	conv_vcard("data/vcards/kdepim/kdepim-umlaute-3.0.vcf", "kde");
}
END_TEST

START_TEST (comparvformat_mismatch1)
{
	compare_vcard("data/vcards/evolution2/compare/1-different.vcf", "data/vcards/kdepim/compare/1-different.vcf", CONV_DATA_MISMATCH);
}
END_TEST

START_TEST (comparvformat_similar1)
{
	compare_vcard("data/vcards/evolution2/compare/1-conflict.vcf", "data/vcards/kdepim/compare/1-conflict.vcf", CONV_DATA_SIMILAR);
}
END_TEST

START_TEST (comparvformat_mismatch2)
{
	compare_vcard("data/vcards/evolution2/compare/2-conflict.vcf", "data/vcards/kdepim/compare/2-conflict.vcf", CONV_DATA_MISMATCH);
}
END_TEST

START_TEST (comparvformat_similar2)
{
	compare_vcard("data/vcards/evolution2/compare/2-different.vcf", "data/vcards/kdepim/compare/2-different.vcf", CONV_DATA_SIMILAR);
}
END_TEST

START_TEST (comparvformat_same1)
{
	compare_vcard("data/vcards/evolution2/compare/1-same.vcf", "data/vcards/kdepim/compare/1-same.vcf", CONV_DATA_SAME);
}
END_TEST

START_TEST (comparvformat_same2)
{
	compare_vcard("data/vcards/evolution2/compare/2-same.vcf", "data/vcards/kdepim/compare/2-same.vcf", CONV_DATA_SAME);
}
END_TEST


Suite *vcard_suite(void)
{
	Suite *s = suite_create("Vcard");
	//Suite *s2 = suite_create("Vcard");
	
	create_case(s, "conv_vcard_evolution2_full1", conv_vcard_evolution2_full1);
	create_case(s, "conv_vcard_evolution2_full2", conv_vcard_evolution2_full2);
	create_case(s, "conv_vcard_evolution2_photo", conv_vcard_evolution2_photo);
	create_case(s, "conv_vcard_evolution2_multiline", conv_vcard_evolution2_multiline);
	create_case(s, "conv_vcard_evolution2_umlaute", conv_vcard_evolution2_umlaute);
	create_case(s, "conv_vcard_evolution2_special", conv_vcard_evolution2_special);
	
	create_case(s, "conv_vcard_kde_21_full1", conv_vcard_kde_21_full1);
	create_case(s, "conv_vcard_kde_30_full1", conv_vcard_kde_30_full1);
	create_case(s, "conv_vcard_kde_21_full2", conv_vcard_kde_21_full2);
	create_case(s, "conv_vcard_kde_30_full2", conv_vcard_kde_30_full2);
	create_case(s, "conv_vcard_kde_21_multiline", conv_vcard_kde_21_multiline);
	create_case(s, "conv_vcard_kde_30_multiline", conv_vcard_kde_30_multiline);
	create_case(s, "conv_vcard_kde_21_photo1", conv_vcard_kde_21_photo1);
	create_case(s, "conv_vcard_kde_30_photo1", conv_vcard_kde_30_photo1);
	create_case(s, "conv_vcard_kde_21_photo2", conv_vcard_kde_21_photo2);
	create_case(s, "conv_vcard_kde_30_photo2", conv_vcard_kde_30_photo2);
	create_case(s, "conv_vcard_kde_21_sound1", conv_vcard_kde_21_sound1);
	create_case(s, "conv_vcard_kde_30_sound1", conv_vcard_kde_30_sound1);
	create_case(s, "conv_vcard_kde_21_sound2", conv_vcard_kde_21_sound2);
	create_case(s, "conv_vcard_kde_30_sound2", conv_vcard_kde_30_sound2);
	create_case(s, "conv_vcard_kde_21_special", conv_vcard_kde_21_special);
	create_case(s, "conv_vcard_kde_30_special", conv_vcard_kde_30_special);
	create_case(s, "conv_vcard_kde_21_umlaute", conv_vcard_kde_21_umlaute);
	create_case(s, "conv_vcard_kde_30_umlaute", conv_vcard_kde_30_umlaute);

	create_case(s, "comparvformat_mismatch1", comparvformat_mismatch1);
	create_case(s, "comparvformat_mismatch2", comparvformat_mismatch2);
	create_case(s, "comparvformat_similar1", comparvformat_similar1);
	create_case(s, "comparvformat_similar2", comparvformat_similar2);
	create_case(s, "comparvformat_same1", comparvformat_same1);
	create_case(s, "comparvformat_same2", comparvformat_same2);
	
	return s;
}

int main(void)
{
	int nf;

	Suite *s = vcard_suite();
	
	SRunner *sr;
	sr = srunner_create(s);
	srunner_run_all(sr, CK_NORMAL);
	nf = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}