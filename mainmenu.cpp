#include "stdafx.h"
#include "pubnubtools.h"
#include <thread>


static const GUID g_mainmenu_group_id = { 0x44963e7a, 0x4b2a, 0x4588, { 0xb0, 0x17, 0xa8, 0x69, 0x18, 0xcb, 0x8a, 0xa5 } };

static mainmenu_group_popup_factory g_mainmenu_group(g_mainmenu_group_id, mainmenu_groups::file, mainmenu_commands::sort_priority_dontcare, "FooQueue");

class mainmenu_commands_sample : public mainmenu_commands {
public:
	enum {
		cmd_library,
		cmd_total
	};

	t_uint32 get_command_count() {
		return cmd_total;
	}
	GUID get_command(t_uint32 p_index) {
		static const GUID guid_getlibrary = { 0x7c23644d, 0x7d31, 0x450d, { 0xb6, 0x56, 0xfc, 0x79, 0x44, 0xe3, 0xa4, 0x93 } };

		switch(p_index) {
			case cmd_library: return guid_getlibrary;
			default: uBugCheck(); // should never happen unless somebody called us with invalid parameters - bail
		}
	}
	void get_name(t_uint32 p_index,pfc::string_base & p_out) {
		switch(p_index) {
			case cmd_library: p_out = "Subscribe To Channel"; break;
			default: uBugCheck(); // should never happen unless somebody called us with invalid parameters - bail
		}
	}
	bool get_description(t_uint32 p_index,pfc::string_base & p_out) {
		switch(p_index) {
			case cmd_library: p_out = "Subscribes to PubNub Channel."; return true;
			default: uBugCheck(); // should never happen unless somebody called us with invalid parameters - bail
		}
	}
	GUID get_parent() {
		return g_mainmenu_group_id;
	}
	void execute(t_uint32 p_index,service_ptr_t<service_base> p_callback) {
		//Not sure if this is the best way of doing this...cannot create these in the switch statement
		PubNubTools pub;
		std::thread t;
		unsigned int myCounter = 0;
		int a = 0;
				
		switch(p_index) {
			case cmd_library:
				t = std::thread(&PubNubTools::PubNubSubscribe, pub);
				t.detach();
				break;
			default:
				uBugCheck(); // should never happen unless somebody called us with invalid parameters - bail
		}
	}

private:	
	
};

static mainmenu_commands_factory_t<mainmenu_commands_sample> g_mainmenu_commands_sample_factory;
