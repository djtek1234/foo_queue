#include "stdafx.h"
#include "resource.h"
#include <string>
#include "../ATLHelpers/ATLHelpers.h"
#include "guid.h"

class CMyPreferences : public CDialogImpl<CMyPreferences>, public preferences_page_instance {
public:
	//Constructor - invoked by preferences_page_impl helpers - don't do Create() in here, preferences_page_impl does this for us
	CMyPreferences(preferences_page_callback::ptr callback) : m_callback(callback) {}
	//Note that we don't bother doing anything regarding destruction of our class.
	//The host ensures that our dialog is destroyed first, then the last reference to our preferences_page_instance object is released, causing our object to be deleted.


	//dialog resource ID
	enum { IDD = IDD_MYPREFERENCES };
	// preferences_page_instance methods (not all of them - get_wnd() is supplied by preferences_page_impl helpers)
	t_uint32 get_state();
	void apply();
	void reset();

	//WTL message map
	BEGIN_MSG_MAP(CMyPreferences)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_HANDLER_EX(IDC_PUBLISH, EN_CHANGE, OnPublishTextChange)
		COMMAND_HANDLER_EX(IDC_CHANNEL, EN_CHANGE, OnChannelTextChange)
		COMMAND_HANDLER_EX(IDC_SUBSCRIBE, EN_CHANGE, OnSubscribeTextChange)
		COMMAND_HANDLER_EX(IDC_ORIGIN, EN_CHANGE, OnOriginTextChange)
		COMMAND_HANDLER_EX(IDC_BUTTON1, BN_CLICKED, OnButton1Click)
		COMMAND_HANDLER_EX(IDC_BUTTON2, BN_CLICKED, OnButton2Click)
	END_MSG_MAP()
private:
	BOOL OnInitDialog(CWindow, LPARAM);
	void OnEditChange(UINT, int, CWindow);
	void OnChannelTextChange(UINT, int, CWindow);
	void OnPublishTextChange(UINT, int, CWindow);
	void OnSubscribeTextChange(UINT, int, CWindow);
	void OnOriginTextChange(UINT, int, CWindow);
	bool HasChanged();
	void OnChanged();
	pfc::string8 getGenreList();
	pfc::string8 getChannelName();
	pfc::string8 getPublishKey();
	pfc::string8 getSubscribeKey();
	pfc::string8 getOrigin();

	void OnButton1Click(UINT, int, CWindow);
	void OnButton2Click(UINT, int, CWindow);

	const preferences_page_callback::ptr m_callback;
};

extern cfg_string cfg_channel;
extern cfg_string cfg_publishkey;
extern cfg_string cfg_subscribekey;
extern cfg_string cfg_origin;