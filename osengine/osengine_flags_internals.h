
/*! @brief The functions that can be called on a plugin */
struct MSyncFlag {
	osync_bool is_set;
	osync_bool is_changing;
	osync_bool default_val;
	MSyncFlag *comb_flag;
	unsigned int num_not_set;
	unsigned int num_set;
	osync_bool is_comb;
	MSyncFlagTriggerFunc pos_trigger_func;
	void *pos_user_data1;
	void *pos_user_data2;
	MSyncFlagTriggerFunc neg_trigger_func;
	void *neg_user_data1;
	void *neg_user_data2;
	osync_bool is_any;
};

MSyncFlag *osync_flag_new(MSyncFlag *parent);
MSyncFlag *osync_comb_flag_new(osync_bool any, osync_bool default_val);
void osync_flag_set_pos_trigger(MSyncFlag *flag, MSyncFlagTriggerFunc func, void *data1, void *data2);
void osync_flag_set_neg_trigger(MSyncFlag *flag, MSyncFlagTriggerFunc func, void *data1, void *data2);
void osync_flag_calculate_comb(MSyncFlag *flag);
osync_bool osync_flag_is_set(MSyncFlag *flag);
osync_bool osync_flag_is_not_set(MSyncFlag *flag);
void osync_comb_flag_update(MSyncFlag *combflag, MSyncFlag *flag, osync_bool prev_state);
void osync_flag_changing(MSyncFlag *flag);
void osync_flag_cancel(MSyncFlag *flag);
void osync_flag_unset(MSyncFlag *flag);
void osync_flag_set(MSyncFlag *flag);
void osync_flag_calc_trigger(MSyncFlag *flag, osync_bool oldstate);
void osync_change_flags_detach(OSyncChange *change);
osync_bool osync_flag_get_state(MSyncFlag *flag);
void osync_flag_free(MSyncFlag *flag);
void osync_flag_set_state(MSyncFlag *flag, osync_bool state);
void osync_flag_attach(MSyncFlag *flag, MSyncFlag *target);
void osync_flag_detach(MSyncFlag *flag);