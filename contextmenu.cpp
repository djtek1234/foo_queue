#include "stdafx.h"


// Identifier of our context menu group. Substitute with your own when reusing code.
static const GUID guid_mygroup = { 0x924c5dd3, 0xdeac, 0x4a28, { 0xb5, 0xe3, 0x4f, 0x6d, 0xb7, 0xc6, 0xaa, 0x2c } };

// Switch to contextmenu_group_factory to embed your commands in the root menu but separated from other commands.
static contextmenu_group_popup_factory g_mygroup(guid_mygroup, contextmenu_groups::root, "FooQueue", 0);

static void RunTestCommand(metadb_handle_list_cref data);

// Simple context menu item class.
class myitem : public contextmenu_item_simple {
public:
	enum {
		cmd_test1 = 0,
		cmd_total
	};
	GUID get_parent() {return guid_mygroup;}
	unsigned get_num_items() {return cmd_total;}
	void get_item_name(unsigned p_index,pfc::string_base & p_out) {
		switch(p_index) {
			case cmd_test1: p_out = "Ban Song"; break;
			default: uBugCheck(); // should never happen unless somebody called us with invalid parameters - bail
		}
	}
	void context_command(unsigned p_index,metadb_handle_list_cref p_data,const GUID& p_caller) {
		switch(p_index) {
			case cmd_test1:
				RunTestCommand(p_data);
				break;
			default:
				uBugCheck();
		}
	}
	// Overriding this is not mandatory. We're overriding it just to demonstrate stuff that you can do such as context-sensitive menu item labels.
	bool context_get_display(unsigned p_index,metadb_handle_list_cref p_data,pfc::string_base & p_out,unsigned & p_displayflags,const GUID & p_caller) {
		switch(p_index) {
			case cmd_test1:
				if (!__super::context_get_display(p_index, p_data, p_out, p_displayflags, p_caller)) return false;
				p_out << " : " << p_data.get_count() << " item";
				if (p_data.get_count() != 1) p_out << "s";
				p_out << " selected";
				return true;
			default: uBugCheck();
		}
	}
	GUID get_item_guid(unsigned p_index) {
		// These GUIDs identify our context menu items. Substitute with your own GUIDs when reusing code.
		static const GUID guid_test1 = { 0xf271941c, 0x9f4, 0x4031, { 0x84, 0x62, 0xa9, 0x57, 0xd7, 0xaa, 0xa5, 0x3 } };

		switch(p_index) {
			case cmd_test1: return guid_test1;
			default: uBugCheck(); // should never happen unless somebody called us with invalid parameters - bail
		}

	}
	bool get_item_description(unsigned p_index,pfc::string_base & p_out) {
		switch(p_index) {
			case cmd_test1:
				p_out = "Banned Song List";
				return true;

			default:
				uBugCheck(); // should never happen unless somebody called us with invalid parameters - bail
		}
	}
};

static contextmenu_item_factory_t<myitem> g_myitem_factory;


static void RunTestCommand(metadb_handle_list_cref data) {
	pfc::string_formatter message;
	message << "Banned song list.\n";
	if (data.get_count() > 0) {
		message << "Parameters:\n";
		for(t_size walk = 0; walk < data.get_count(); ++walk) {
			message << data[walk] << "\n";
		}
	}	
	popup_message::g_show(message, "Banned song list:");
}
