#include "preferences.h"


//Default CFG values
static pfc::string8 default_cfg_genrelist = "New Country, Classic Country, New Pop, Classic Pop, New Rock, Classic Rock";
static pfc::string8 default_cfg_bannedlist = "";
static pfc::string8 default_cfg_channel = "";
static pfc::string8 default_cfg_publishkey = "";
static pfc::string8 default_cfg_subscribekey = "";
static pfc::string8 default_cfg_origin = "http://pubsub.pubnub.com";


//CFG instances
static cfg_string cfg_genrelist(guid_cfg_genrelist, default_cfg_genrelist);
static cfg_string cfg_bannedList(guid_cfg_bannedlist, default_cfg_bannedlist);
cfg_string cfg_channel(guid_cfg_channel, default_cfg_channel);
cfg_string cfg_publishkey(guid_cfg_publishkey, default_cfg_publishkey);
cfg_string cfg_subscribekey(guid_cfg_subscribekey, default_cfg_subscribekey);
cfg_string cfg_origin(guid_cfg_origin, default_cfg_origin);


BOOL CMyPreferences::OnInitDialog(CWindow, LPARAM) {
	SetDlgItemText(IDC_CHANNEL, (CString)cfg_channel);
	SetDlgItemText(IDC_PUBLISH, (CString)cfg_publishkey);
	SetDlgItemText(IDC_SUBSCRIBE, (CString)cfg_subscribekey);
	SetDlgItemText(IDC_ORIGIN, (CString)cfg_origin);

	CString genrelist[] = { "New Country", "Classic Country", "New Pop", "Classic Pop", "New Rock", "Classic Rock" };
	pfc::string8 gList = cfg_genrelist;
	pfc::stringcvt::string_wide_from_utf8 uList(gList);
	CString cList = (CString)uList;

	LPTSTR  lpBuffer;
	int count = 0;


	HWND hwndList = GetDlgItem(IDC_LIST1);
	HWND hwndList2 = GetDlgItem(IDC_LIST2);

	for (int i = 0; i < ARRAYSIZE(genrelist); i++)
	{
		//pfc::stringcvt::string_utf8_from_wide test(genrelist[i]);
		int index = cList.Find(genrelist[i]);
		if (index != -1)
		{
			lpBuffer = genrelist[i].GetBuffer(genrelist[i].GetLength());
			int pos = (int)SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)lpBuffer);
			SendMessage(hwndList, LB_SETITEMDATA, pos, (LPARAM)i);
		}
		else
		{
			lpBuffer = genrelist[i].GetBuffer(genrelist[i].GetLength());
			int pos = (int)SendMessage(hwndList2, LB_ADDSTRING, 0, (LPARAM)lpBuffer);
			SendMessage(hwndList2, LB_SETITEMDATA, pos, (LPARAM)i);

		}
		
	}

	return FALSE;
}

//Move selected items from allowed to ban listbox
void CMyPreferences::OnButton1Click(UINT, int, CWindow) {
	HWND hwndList = GetDlgItem(IDC_LIST1);
	HWND hwndList2 = GetDlgItem(IDC_LIST2);
	int pos;
	LPTSTR  lpBuffer;

	CString gValue;

	CListBox list = (CListBox)hwndList;
	CListBox list2 = (CListBox)hwndList2;

	if (list.GetCurSel() >= 0)
	{
		list.GetText(list.GetCurSel(), gValue);

		lpBuffer = gValue.GetBuffer(gValue.GetLength());

		//Add genre to the banned list
		SendMessage(hwndList2, LB_ADDSTRING, 0, (LPARAM)lpBuffer);

		//Remove from the allowed list
		pos = (int)SendMessage(hwndList, LB_GETCURSEL, 0, 0L);
		SendMessage(hwndList, LB_DELETESTRING, pos, 0L);
	}
	else
	{

	}

	cfg_genrelist = getGenreList();
	
}

void CMyPreferences::OnButton2Click(UINT, int, CWindow) {
	HWND hwndList = GetDlgItem(IDC_LIST1);
	HWND hwndList2 = GetDlgItem(IDC_LIST2);

	int pos;
	LPTSTR  lpBuffer;

	CString gValue;

	CListBox list = (CListBox)hwndList;
	CListBox list2 = (CListBox)hwndList2;

	if (list2.GetCurSel() >= 0)
	{
		list2.GetText(list2.GetCurSel(), gValue);

		lpBuffer = gValue.GetBuffer(gValue.GetLength());

		//Add genre back into allowed list
		SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)lpBuffer);

		//Remove from the banned list
		pos = (int)SendMessage(hwndList2, LB_GETCURSEL, 0, 0L);
		SendMessage(hwndList2, LB_DELETESTRING, pos, 0L);
	}
	else
	{

	}

	cfg_genrelist = getGenreList();

}


void CMyPreferences::OnEditChange(UINT, int, CWindow) {
	// not much to do here
	OnChanged();
}

void CMyPreferences::OnChannelTextChange(UINT, int, CWindow) {
	cfg_channel = getChannelName();	
	OnChanged();
}

void CMyPreferences::OnPublishTextChange(UINT, int, CWindow) {
	cfg_publishkey = getPublishKey();
	OnChanged();
}

void CMyPreferences::OnSubscribeTextChange(UINT, int, CWindow) {
	cfg_subscribekey = getSubscribeKey();
	OnChanged();
}

void CMyPreferences::OnOriginTextChange(UINT, int, CWindow) {
	cfg_origin = getOrigin();
	OnChanged();
}

t_uint32 CMyPreferences::get_state() {
	t_uint32 state = preferences_state::resettable;
	if (HasChanged()) state |= preferences_state::changed;
	return state;
}

void CMyPreferences::reset() {
	SetDlgItemText(IDC_CHANNEL, (CString)default_cfg_channel);
	SetDlgItemText(IDC_PUBLISH, (CString)default_cfg_publishkey);
	SetDlgItemText(IDC_SUBSCRIBE, (CString)default_cfg_subscribekey);
	SetDlgItemText(IDC_ORIGIN, (CString)default_cfg_origin);
	//SetDlgItemText( (IDC_LIST1, default_cfg_genrelist, FALSE);
	OnChanged();
}

void CMyPreferences::apply() {
	cfg_channel = getSubscribeKey();
	cfg_publishkey = getPublishKey();
	cfg_subscribekey = getSubscribeKey();
	cfg_genrelist = getGenreList();
	cfg_origin = getOrigin();
	//cfg_allowedList = GetDlgItemInt(IDC_LIST1, NULL, FALSE);
	//cfg_bannedList = GetDlgItemInt(IDC_LIST2, NULL, FALSE);

	OnChanged(); //our dialog content has not changed but the flags have - our currently shown values now match the settings so the apply button can be disabled
}

pfc::string8 CMyPreferences::getGenreList(){
	HWND hwndList = GetDlgItem(IDC_LIST1);

	CListBox list = (CListBox)hwndList;
	pfc::string8 selected = "";
	
	CString space = " ";

	int n;

	for (int i = 0; i < list.GetCount(); i++)
	{
		CString temp;
		n = list.GetTextLen(i);
		list.GetText(i, temp.GetBuffer(n));
		temp.ReleaseBuffer();
		temp.Append(space);
		pfc::stringcvt::string_utf8_from_wide test2(temp);
		selected.add_string(test2);
	}

	return selected;
}

pfc::string8 CMyPreferences::getChannelName(){
	pfc::string8 temp;

	uGetDlgItemText(m_hWnd, IDC_CHANNEL, temp);
	
	return temp;
}

pfc::string8 CMyPreferences::getPublishKey(){
	pfc::string8 temp;

	uGetDlgItemText(m_hWnd, IDC_PUBLISH, temp);

	return temp;
}

pfc::string8 CMyPreferences::getSubscribeKey(){
	pfc::string8 temp;

	uGetDlgItemText(m_hWnd, IDC_SUBSCRIBE, temp);

	return temp;
}

pfc::string8 CMyPreferences::getOrigin(){
	pfc::string8 temp;

	uGetDlgItemText(m_hWnd, IDC_ORIGIN, temp);

	return temp;
}



bool CMyPreferences::HasChanged() {
	//returns whether our dialog content is different from the current configuration (whether the apply button should be enabled or not)
	return getChannelName() != cfg_channel || getPublishKey() != cfg_publishkey || getSubscribeKey() != cfg_subscribekey || getGenreList() != cfg_genrelist || getOrigin() != cfg_origin;
}
void CMyPreferences::OnChanged() {
	//tell the host that our state has changed to enable/disable the apply button appropriately.
	m_callback->on_state_changed();
}

class preferences_page_myimpl : public preferences_page_impl<CMyPreferences> {
	// preferences_page_impl<> helper deals with instantiation of our dialog; inherits from preferences_page_v3.
public:
	const char * get_name() {return "FooQueue";}
	GUID get_guid() {
		// This is our GUID. Replace with your own when reusing the code.
		static const GUID guid = { 0xff195f10, 0xde86, 0x476e, { 0xae, 0x4c, 0x1d, 0xe6, 0x95, 0xd2, 0x41, 0x48 } };
		return guid;
	}
	GUID get_parent_guid() {return guid_tools;}
};

static preferences_page_factory_t<preferences_page_myimpl> g_preferences_page_myimpl_factory;
