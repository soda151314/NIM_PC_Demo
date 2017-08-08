﻿#include "profile_form.h"
#include "callback/multiport/multiport_push_callback.h"
#include "module/session/session_manager.h"
#include "module/service/mute_black_service.h"
#include "module/video/video_manager.h"
#include "head_modify_form.h"
#include "api/nim_cpp_friend.h"
#include "callback/team/team_callback.h"

namespace nim_comp
{
const LPCTSTR ProfileForm::kClassName = L"ProfileForm";

ProfileForm * ProfileForm::ShowProfileForm(UTF8String uid, bool is_robot/* = false*/)
{
	ProfileForm* form = (ProfileForm*)WindowsManager::GetInstance()->GetWindow(kClassName, kClassName);
	if (form != NULL && form->m_uinfo.GetAccId().compare(uid) == 0) //当前已经打开的名片正是希望打开的名片
		; // 直接显示
	else
	{
		if (form != NULL && form->m_uinfo.GetAccId().compare(uid) != 0)//当前已经打开的名片不是希望打开的名片
			::DestroyWindow(form->m_hWnd); //关闭重新创建
		form = new ProfileForm();
		form->Create(NULL, L"", WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0L);
		
		if (!is_robot)
		{
			// 获取用户信息
			nim::UserNameCard info;
			UserService::GetInstance()->GetUserInfo(uid, info);
			form->InitUserInfo(info);
		}
		else
		{
			nim::RobotInfo info;
			UserService::GetInstance()->GetRobotInfo(uid, info);
			form->InitRobotInfo(info);
		}
	}
	if (!::IsWindowVisible(form->m_hWnd))
	{
		::ShowWindow(form->m_hWnd, SW_SHOW);
		form->CenterWindow();
	}
	form->__super::ActiveWindow();
	return form;
}

ProfileForm *ProfileForm::ShowProfileForm(UTF8String tid, UTF8String uid, nim::NIMTeamUserType my_type)
{
	ProfileForm* form = (ProfileForm*)WindowsManager::GetInstance()->GetWindow(kClassName, kClassName);
	if (form != NULL && form->m_uinfo.GetAccId().compare(uid) == 0 && form->tid_ == tid) //当前已经打开的名片正是希望打开的名片
		; // 直接显示
	else
	{
		if (form != NULL && (form->m_uinfo.GetAccId().compare(uid) != 0 || form->tid_ != tid))//当前已经打开的名片不是希望打开的名片
			::DestroyWindow(form->m_hWnd); //关闭重新创建
		form = new ProfileForm(tid, uid, my_type);
		form->Create(NULL, L"", WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0L);

		// 获取用户信息
		nim::UserNameCard info;
		UserService::GetInstance()->GetUserInfo(uid, info);
		form->InitUserInfo(info);
	}
	if (!::IsWindowVisible(form->m_hWnd))
	{
		::ShowWindow(form->m_hWnd, SW_SHOW);
		form->CenterWindow();
	}
	form->__super::ActiveWindow();
	return form;
}

ProfileForm::ProfileForm()
{
	auto user_info_change_cb = nbase::Bind(&ProfileForm::OnUserInfoChange, this, std::placeholders::_1);
	unregister_cb.Add(UserService::GetInstance()->RegUserInfoChange(user_info_change_cb));

	auto user_photo_ready_cb = nbase::Bind(&ProfileForm::OnUserPhotoReady, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	unregister_cb.Add(PhotoService::GetInstance()->RegPhotoReady(user_photo_ready_cb));

	auto misc_uinfo_change_cb = nbase::Bind(&ProfileForm::OnMiscUInfoChange, this, std::placeholders::_1);
	unregister_cb.Add(UserService::GetInstance()->RegMiscUInfoChange(misc_uinfo_change_cb));

	auto friend_list_change_cb = nbase::Bind(&ProfileForm::OnFriendListChange, this, std::placeholders::_1, std::placeholders::_2);
	unregister_cb.Add(UserService::GetInstance()->RegFriendListChange(friend_list_change_cb));

	auto robot_list_change_cb = nbase::Bind(&ProfileForm::OnRobotChange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	unregister_cb.Add(UserService::GetInstance()->RegRobotListChange(robot_list_change_cb));
}

ProfileForm::~ProfileForm()
{

}

std::wstring ProfileForm::GetSkinFolder()
{
	return L"profile_form";
}

std::wstring ProfileForm::GetSkinFile()
{
	return L"profile_form.xml";
}

std::wstring ProfileForm::GetWindowClassName() const
{
	return kClassName;
}

std::wstring ProfileForm::GetWindowId() const
{
	return kClassName;
}

void ProfileForm::InitWindow()
{
	m_pRoot->AttachBubbledEvent(ui::kEventTab, nbase::Bind(&ProfileForm::Notify, this, std::placeholders::_1));

	head_image_btn = static_cast<ui::Button*>(FindControl(L"head_image"));

	show_name_label = static_cast<ui::Label*>(FindControl(L"show_name"));
	user_id_label = static_cast<ui::Label*>(FindControl(L"userid"));
	nickname_label = static_cast<ui::Label*>(FindControl(L"nickname"));
	sex_icon = static_cast<ui::CheckBox*>(FindControl(L"sex_icon"));

	multi_push_switch = static_cast<ui::CheckBox*>(FindControl(L"multi_push_switch"));
	webrtc_setting_ = static_cast<ui::CheckBox*>(FindControl(L"webrtc_setting"));
	notify_switch = static_cast<ui::CheckBox*>(FindControl(L"notify_switch"));
	black_switch = static_cast<ui::CheckBox*>(FindControl(L"black_switch"));
	mute_switch = static_cast<ui::CheckBox*>(FindControl(L"mute_switch"));
	start_chat = static_cast<ui::Button*>(FindControl(L"start_chat"));
	add_or_del = static_cast<ui::TabBox*>(FindControl(L"add_or_del"));
	delete_friend = static_cast<ui::Button*>(FindControl(L"delete_friend"));
	add_friend = static_cast<ui::Button*>(FindControl(L"add_friend"));

	btn_modify_info = static_cast<ui::Button*>(FindControl(L"modify_info"));
	btn_cancel_modify = static_cast<ui::Button*>(FindControl(L"cancel_modify"));
	btn_save_modify = static_cast<ui::Button*>(FindControl(L"save_modify"));
	
	alias_box = static_cast<ui::HBox*>(FindControl(L"alias_box"));
	alias_edit = static_cast<ui::RichEdit*>(FindControl(L"alias_edit"));
	
	nickname_box = static_cast<ui::HBox*>(FindControl(L"nickname_box"));
	nickname_edit = static_cast<ui::RichEdit*>(FindControl(L"nickname_edit"));
	nickname_error_tip = static_cast<ui::Label*>(FindControl(L"nickname_error_tip"));
	
	sex_box = static_cast<ui::HBox*>(FindControl(L"sex_box"));
	sex_option = static_cast<ui::Combo*>(FindControl(L"sex_option"));
	
	birthday_label = static_cast<ui::Label*>(FindControl(L"birthday"));
	birthday_combo_box = static_cast<ui::HBox*>(FindControl(L"birthday_combobox"));
	birth_year_combo = static_cast<ui::Combo*>(birthday_combo_box->FindSubControl(L"year"));
	birth_month_combo = static_cast<ui::Combo*>(birthday_combo_box->FindSubControl(L"month"));
	birth_day_combo = static_cast<ui::Combo*>(birthday_combo_box->FindSubControl(L"day"));
	
	phone_label = static_cast<ui::Label*>(FindControl(L"phone"));
	phone_edit = static_cast<ui::RichEdit*>(FindControl(L"phone_edit"));
	
	email_label = static_cast<ui::Label*>(FindControl(L"email"));
	email_edit = static_cast<ui::RichEdit*>(FindControl(L"email_edit"));
	
	signature_label = static_cast<ui::Label*>(FindControl(L"signature"));
	signature_edit = static_cast<ui::RichEdit*>(FindControl(L"signature_edit"));

	common_info_ = static_cast<ui::VBox*>(FindControl(L"common_info"));
	robot_info_ = static_cast<ui::VBox*>(FindControl(L"robot_info"));
	common_other_ = static_cast<ui::VBox*>(FindControl(L"common_other"));
	robot_intro_ = static_cast<ui::RichEdit*>(FindControl(L"robot_intro"));

	if (tid_.empty())
	{
		ui::Box* panel = static_cast<ui::Box*>(FindControl(L"mute_switch_box"));
		panel->SetVisible(false);
	}
	else
	{
		if (my_team_user_type_ != nim::kNIMTeamUserTypeCreator && my_team_user_type_ != nim::kNIMTeamUserTypeManager)
			mute_switch->SetEnabled(false);
		else
			have_mute_right_ = true;
	}
}

void ProfileForm::InitRobotInfo(const nim::RobotInfo & info)
{
	m_robot = info;
	common_info_->SetVisible(false);
	common_other_->SetVisible(false);
	sex_icon->SetVisible(false);
	FindControl(L"only_me")->SetVisible(false);
	robot_info_->SetVisible(true);
	user_id_label->SetUTF8Text(info.GetAccid());
	show_name_label->SetUTF8Text(info.GetName());
	robot_intro_->SetText(nbase::UTF8ToUTF16(info.GetIntro()));
	head_image_btn->SetEnabled(false);
	head_image_btn->SetBkImage(PhotoService::GetInstance()->GetUserPhoto(info.GetAccid(), true));
	add_or_del->SetVisible(false);
	start_chat->AttachClick(nbase::Bind(&ProfileForm::OnStartChatBtnClicked, this, std::placeholders::_1));

	ui::MutiLanSupport* mls = ui::MutiLanSupport::GetInstance();
	std::wstring title = nbase::StringPrintf(mls->GetStringViaID(L"STRID_PROFILE_FORM_WHOSE_NAMECARD").c_str(), nbase::UTF8ToUTF16(info.GetName()).c_str());
	SetTaskbarTitle(title); //任务栏标题
}

void ProfileForm::InitUserInfo(const nim::UserNameCard &info)
{
	m_uinfo = info;

	if (m_uinfo.GetAccId() == LoginManager::GetInstance()->GetAccount())
		user_type = -1;
	else
		user_type = UserService::GetInstance()->GetUserType(m_uinfo.GetAccId());

	if (user_type == -1) // 自己的名片
	{
		// 获取多端推送开关
		nim::Client::GetMultiportPushConfigAsync(&MultiportPushCallback::OnMultiportPushConfigChange);

		head_image_btn->SetMouseEnabled(true); // 可点击头像进行更换
		btn_modify_info->SetVisible(true); // 显示“编辑”按钮
		head_image_btn->SetMouseEnabled(true); // 可点击头像进行更换

		FindControl(L"only_other")->SetVisible(false);	// 当名片是自己的时候，隐藏下面两块
		FindControl(L"only_me")->SetVisible(true);		// 当名片是自己的时候，显示多端推送开关

		nickname_edit->SetLimitText(10);
		phone_edit->SetLimitText(13);
		email_edit->SetLimitText(30);
		signature_edit->SetLimitText(30);

		head_image_btn->AttachClick(nbase::Bind(&ProfileForm::OnHeadImageClicked, this, std::placeholders::_1));

		birth_year_combo->AttachSelect(nbase::Bind(&ProfileForm::OnBirthdayComboSelect, this, std::placeholders::_1));
		birth_month_combo->AttachSelect(nbase::Bind(&ProfileForm::OnBirthdayComboSelect, this, std::placeholders::_1));

		btn_modify_info->AttachClick(nbase::Bind(&ProfileForm::OnModifyOrCancelBtnClicked, this, std::placeholders::_1, true));
		btn_cancel_modify->AttachClick(nbase::Bind(&ProfileForm::OnModifyOrCancelBtnClicked, this, std::placeholders::_1, false));
		btn_save_modify->AttachClick(nbase::Bind(&ProfileForm::OnSaveInfoBtnClicked, this, std::placeholders::_1));

		multi_push_switch->AttachSelect(nbase::Bind(&ProfileForm::OnMultiPushSwitchSelected, this, std::placeholders::_1));
		multi_push_switch->AttachUnSelect(nbase::Bind(&ProfileForm::OnMultiPushSwitchUnSelected, this, std::placeholders::_1));
		webrtc_setting_->Selected(nim_comp::VideoManager::GetInstance()->GetWebrtc(), false);
		webrtc_setting_->AttachSelect(nbase::Bind(&ProfileForm::OnWebRtcSelected, this, std::placeholders::_1));
		webrtc_setting_->AttachUnSelect(nbase::Bind(&ProfileForm::OnWebRtcUnSelected, this, std::placeholders::_1));
	}
	else
	{
		FindControl(L"only_other")->SetVisible(true);	// 当名片是自己的时候，显示下面两块
		FindControl(L"only_me")->SetVisible(false);		// 当名片是自己的时候，隐藏多端推送开关

		CheckInMuteBlack();
		add_or_del->SelectItem(user_type == nim::kNIMFriendFlagNormal ? 0 : 1);

		alias_edit->SetLimitText(16);
		alias_edit->SetSelAllOnFocus(true);
		alias_edit->AttachSetFocus(nbase::Bind(&ProfileForm::OnAliasEditGetFocus, this, std::placeholders::_1));
		alias_edit->AttachKillFocus(nbase::Bind(&ProfileForm::OnAliasEditLoseFocus, this, std::placeholders::_1));
		alias_edit->AttachMouseEnter(nbase::Bind(&ProfileForm::OnAliasEditMouseEnter, this, std::placeholders::_1));
		alias_edit->AttachMouseLeave(nbase::Bind(&ProfileForm::OnAliasEditMouseLeave, this, std::placeholders::_1));
		alias_edit->AttachReturn(nbase::Bind(&ProfileForm::OnReturnOnAliasEdit, this, std::placeholders::_1));

		notify_switch->AttachSelect(nbase::Bind(&ProfileForm::OnNotifySwitchSelected, this, std::placeholders::_1));
		notify_switch->AttachUnSelect(nbase::Bind(&ProfileForm::OnNotifySwitchUnSelected, this, std::placeholders::_1));
		black_switch->AttachSelect(nbase::Bind(&ProfileForm::OnBlackSwitchSelected, this, std::placeholders::_1));
		black_switch->AttachUnSelect(nbase::Bind(&ProfileForm::OnBlackSwitchUnSelected, this, std::placeholders::_1));
		mute_switch->AttachSelect(nbase::Bind(&ProfileForm::OnMuteSwitchSelected, this, std::placeholders::_1));
		mute_switch->AttachUnSelect(nbase::Bind(&ProfileForm::OnMuteSwitchUnSelected, this, std::placeholders::_1));

		unregister_cb.Add(MuteBlackService::GetInstance()->RegSyncSetMuteCallback(nbase::Bind(&ProfileForm::OnNotifyChangeCallback, this, std::placeholders::_1, std::placeholders::_2)));
		unregister_cb.Add(MuteBlackService::GetInstance()->RegSyncSetBlackCallback(nbase::Bind(&ProfileForm::OnBlackChangeCallback, this, std::placeholders::_1, std::placeholders::_2)));
		
		start_chat->AttachClick(nbase::Bind(&ProfileForm::OnStartChatBtnClicked, this, std::placeholders::_1));
		delete_friend->AttachClick(nbase::Bind(&ProfileForm::OnDeleteFriendBtnClicked, this, std::placeholders::_1));
		add_friend->AttachClick(nbase::Bind(&ProfileForm::OnAddFriendBtnClicked, this, std::placeholders::_1));

		if (have_mute_right_)
		{
			//TODO(litianyi) 同步堵塞接口测试
			//nim::TeamMemberProperty prop = nim::Team::QueryTeamMemberBlock(tid_, m_uinfo.GetAccId());
			//mute_switch->Selected(prop.IsMute());

			nim::Team::QueryTeamMemberAsync(tid_, m_uinfo.GetAccId(), ToWeakCallback([this](const nim::TeamMemberProperty& team_member_info) {
				mute_switch->Selected(team_member_info.IsMute());
			}));
		}
	}

	InitLabels();
}

void ProfileForm::CheckInMuteBlack()
{
	auto service = MuteBlackService::GetInstance();
	auto mute_list = service->GetMuteList();
	auto black_list = service->GetBlackList();
	notify_switch->Selected(!service->IsInMuteList(m_uinfo.GetAccId()));
	if (service->IsInBlackList(m_uinfo.GetAccId()))
	{
		black_switch->Selected(true);
		notify_switch->SetEnabled(false);
	}
	else
		black_switch->Selected(false);
}

bool ProfileForm::Notify(ui::EventArgs * msg)
{
	std::wstring name = msg->pSender->GetName();

	if (msg->Type == ui::kEventTab)
	{
		if (btn_modify_info->IsVisible()) // 当前不是编辑页面
			return false;

		if (name == L"nickname_edit")
		{
			phone_edit->SetFocus();
		}
		else if (name == L"phone_edit")
		{
			email_edit->SetFocus();
		}
		else if (name == L"email_edit")
		{
			signature_edit->SetFocus();
		}
		else if (name == L"signature_edit")
		{
			nickname_edit->SetFocus();
		}
	}
	return true;
}

bool ProfileForm::OnMultiPushSwitchSelected(ui::EventArgs* args)
{
	nim::Client::SetMultiportPushConfigAsync(true, &MultiportPushCallback::OnMultiportPushConfigChange);
	return true;
}

bool ProfileForm::OnMultiPushSwitchUnSelected(ui::EventArgs* args)
{
	nim::Client::SetMultiportPushConfigAsync(false, &MultiportPushCallback::OnMultiportPushConfigChange);
	return true;
}
bool ProfileForm::OnWebRtcSelected(ui::EventArgs* args)
{
	nim_comp::VideoManager::GetInstance()->SetWebrtc(true);
	return true;
}

bool ProfileForm::OnWebRtcUnSelected(ui::EventArgs* args)
{
	nim_comp::VideoManager::GetInstance()->SetWebrtc(false);
	return true;
}

bool ProfileForm::OnNotifySwitchSelected(ui::EventArgs* args)
{
	MuteBlackService::GetInstance()->InvokeSetMute(m_uinfo.GetAccId(), false);
	return true;
}

bool ProfileForm::OnNotifySwitchUnSelected(ui::EventArgs* args)
{
	MuteBlackService::GetInstance()->InvokeSetMute(m_uinfo.GetAccId(), true);
	return true;
}

bool ProfileForm::OnBlackSwitchSelected(ui::EventArgs* args)
{
	MuteBlackService::GetInstance()->InvokeSetBlack(m_uinfo.GetAccId(), true);
	return true;
}

bool ProfileForm::OnBlackSwitchUnSelected(ui::EventArgs* args)
{
	MuteBlackService::GetInstance()->InvokeSetBlack(m_uinfo.GetAccId(), false);
	return true;
}

bool ProfileForm::OnMuteSwitchSelected(ui::EventArgs* args)
{
	nim::Team::MuteMemberAsync(tid_, m_uinfo.GetAccId(), true, nbase::Bind(&TeamCallback::OnTeamEventCallback, std::placeholders::_1));
	return true;
}

bool ProfileForm::OnMuteSwitchUnSelected(ui::EventArgs* args)
{
	nim::Team::MuteMemberAsync(tid_, m_uinfo.GetAccId(), false, nbase::Bind(&TeamCallback::OnTeamEventCallback, std::placeholders::_1));
	return true;
}

void ProfileForm::OnNotifyChangeCallback(std::string id, bool mute)
{
	if (id == m_uinfo.GetAccId())
		notify_switch->Selected(!mute);
}

void ProfileForm::OnBlackChangeCallback(std::string id, bool black)
{
	if (id == m_uinfo.GetAccId())
	{
		if (black)
		{
			black_switch->Selected(true);
			notify_switch->SetEnabled(false);
		}
		else
		{
			black_switch->Selected(false);
			notify_switch->SetEnabled(true);
		}
	}
}

bool ProfileForm::OnStartChatBtnClicked(ui::EventArgs* args)
{
	if (!m_uinfo.GetAccId().empty())
		SessionManager::GetInstance()->OpenSessionBox(m_uinfo.GetAccId(), nim::kNIMSessionTypeP2P);
	else
		SessionManager::GetInstance()->OpenSessionBox(m_robot.GetAccid(), nim::kNIMSessionTypeP2P);
	return true;
}

bool ProfileForm::OnDeleteFriendBtnClicked(ui::EventArgs* args)
{
	MsgboxCallback cb = nbase::Bind(&ProfileForm::OnDeleteFriendMsgBox, this, std::placeholders::_1);
	ShowMsgBox(m_hWnd, cb, L"STRID_PROFILE_FORM_DELETE_FRIEND_TIP", true, L"STRING_TIPS", true, L"STRING_OK", true, L"STRING_NO", true);
	return true;
}

void ProfileForm::OnDeleteFriendMsgBox(MsgBoxRet ret)
{
	if (ret == MB_YES)
	{
		nim::Friend::Delete(m_uinfo.GetAccId(), ToWeakCallback([this](int res_code) {
			if (res_code == 200)
				Close();
		}));
	}
}

bool ProfileForm::OnAddFriendBtnClicked(ui::EventArgs* args)
{
	nim::Friend::Request(m_uinfo.GetAccId(), nim::kNIMVerifyTypeAdd, "", ToWeakCallback([this](int res_code) {
		if (res_code == 200)
			Close();
	}));

	return true;
}

void ProfileForm::OnModifyHeaderComplete(const std::string& id, const std::string &url)
{
	// 头像上传成功，开始更新用户信息
	auto update_cb = nbase::Bind(&ProfileForm::UpdateUInfoHeaderCallback, this, std::placeholders::_1);
	UserService::GetInstance()->InvokeUpdateMyPhoto(url, update_cb);
}

void ProfileForm::UpdateUInfoHeaderCallback(int res)
{
	QLOG_ERR(L"updateUInfoHeaderCallback error code: {0}.") << res;
}

bool ProfileForm::OnHeadImageClicked(ui::EventArgs * args)
{
	HeadModifyForm* form = (HeadModifyForm*)WindowsManager::GetInstance()->GetWindow(HeadModifyForm::kClassName, HeadModifyForm::kClassName);
	if (form == NULL)
	{
		form = new HeadModifyForm(m_uinfo.GetAccId(), L"");
		form->Create(NULL, NULL, WS_OVERLAPPED, 0L);
		form->ShowWindow(true);
		form->CenterWindow();
	}
	else
	{
		::SetForegroundWindow(form->GetHWND());
	}
	form->RegCompleteCallback(nbase::Bind(&ProfileForm::OnModifyHeaderComplete, this, std::placeholders::_1, std::placeholders::_2));

	return true;
}

bool ProfileForm::OnModifyOrCancelBtnClicked(ui::EventArgs* args, bool to_modify)
{
	if (LoginManager::GetInstance()->IsEqual(m_uinfo.GetAccId()))
	{
		btn_modify_info->SetVisible(!to_modify);
		btn_save_modify->SetVisible(to_modify);
		btn_cancel_modify->SetVisible(to_modify);
	}
	nickname_box->SetVisible(to_modify);
	sex_box->SetVisible(to_modify);
	birthday_label->SetVisible(!to_modify);
	birthday_combo_box->SetVisible(to_modify);
	phone_label->SetVisible(!to_modify);
	phone_edit->SetVisible(to_modify);
	email_label->SetVisible(!to_modify);
	email_edit->SetVisible(to_modify);
	signature_label->SetVisible(!to_modify);
	signature_edit->SetVisible(to_modify);
	if (to_modify)
	{
		InitBirthdayCombo(); //初始化生日下拉框，在下面设置初始值
		InitEdits();
		nickname_error_tip->SetVisible(false);
	}
	return true;
}

bool ProfileForm::OnSaveInfoBtnClicked(ui::EventArgs * args)
{
	std::wstring nick = nbase::StringTrim(nickname_edit->GetText());
	if (nick.empty())
	{
		nickname_error_tip->SetText(ui::MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_PROFILE_FORM_NICKNAME_NON_EMPTY"));
		nickname_error_tip->SetVisible(true);
		return false;
	}
	else
		nickname_error_tip->SetVisible(false);

	nim::UserNameCard new_info;
	new_info.SetAccId(m_uinfo.GetAccId());

	if (!m_uinfo.ExistValue(nim::kUserNameCardKeyName) || nbase::UTF16ToUTF8(nick) != m_uinfo.GetName())
	{
		new_info.SetName(nbase::UTF16ToUTF8(nick));
	}

	switch (sex_option->GetCurSel())
	{
	case 0:
		if (!m_uinfo.ExistValue(nim::kUserNameCardKeyGender) || m_uinfo.GetGender() != UG_MALE)
		{
			new_info.SetGender(UG_MALE);
		}
		break;
	case 1:
		if (!m_uinfo.ExistValue(nim::kUserNameCardKeyGender) || m_uinfo.GetGender() != UG_FEMALE)
		{
			new_info.SetGender(UG_FEMALE);
		}
		break;
	default:
		break;
	}

	if (birth_year_combo->GetCurSel() != -1 && birth_month_combo->GetCurSel() != -1 && birth_day_combo->GetCurSel() != -1)
	{
		int this_year = nbase::Time::Now().ToTimeStruct(true).year();
		int birth_year = this_year - birth_year_combo->GetCurSel() - 1;
		int birth_month = birth_month_combo->GetCurSel() + 1;
		int birth_day = birth_day_combo->GetCurSel() + 1;
		std::string new_birthday = nbase::StringPrintf("%04d-%02d-%02d", birth_year, birth_month, birth_day);
		if (!m_uinfo.ExistValue(nim::kUserNameCardKeyBirthday) || new_birthday != m_uinfo.GetBirth())
		{
			new_info.SetBirth( nbase::StringPrintf("%04d-%02d-%02d", birth_year, birth_month, birth_day));
		}
	}

	if (!m_uinfo.ExistValue(nim::kUserNameCardKeyMobile) && !phone_edit->GetUTF8Text().empty()
		|| (m_uinfo.ExistValue(nim::kUserNameCardKeyMobile) && phone_edit->GetUTF8Text() != m_uinfo.GetMobile()))
	{
		new_info.SetMobile(nbase::UTF16ToUTF8(phone_edit->GetText()));
	}
	if (!m_uinfo.ExistValue(nim::kUserNameCardKeyEmail) && !email_edit->GetUTF8Text().empty()
		|| (m_uinfo.ExistValue(nim::kUserNameCardKeyEmail) && email_edit->GetUTF8Text() != m_uinfo.GetEmail()))
	{
		new_info.SetEmail(nbase::UTF16ToUTF8(email_edit->GetText()));
	}
	if (!m_uinfo.ExistValue(nim::kUserNameCardKeySignature) && !signature_edit->GetUTF8Text().empty()
		|| (m_uinfo.ExistValue(nim::kUserNameCardKeySignature) && signature_edit->GetUTF8Text() != m_uinfo.GetSignature()))
	{
		new_info.SetSignature(nbase::UTF16ToUTF8(signature_edit->GetText()));
	}
	
	if (new_info.ExistValue(nim::kUserNameCardKeyAll))
		UserService::GetInstance()->InvokeUpdateMyInfo(new_info, nullptr);
	else
		OnModifyOrCancelBtnClicked(nullptr, false);

	return true;
}

bool nim_comp::ProfileForm::OnAliasEditGetFocus(ui::EventArgs * args)
{
	alias_edit->SetBorderSize(1);
	return false;
}

bool ProfileForm::OnAliasEditLoseFocus(ui::EventArgs * args)
{
	alias_edit->SetBorderSize(0);

	if(alias_edit->GetText() == UserService::GetInstance()->GetFriendAlias(m_uinfo.GetAccId()))
		return false;

	nim::FriendProfile profile;
	//Json::Value values;
	//Json::Reader reader;
	//std::string test_string = "{\"remote\":{\"mapmap\":{\"int\":1,\"boolean\":false,\"list\":[1,2,3],\"string\":\"string, lalala\"}}}";
	//if (reader.parse(test_string, values))
	//	profile.SetEx(values);
	profile.SetAccId(m_uinfo.GetAccId());
	profile.SetAlias(alias_edit->GetUTF8Text());
	nim::Friend::Update(profile, nullptr);

	return true;
}

bool ProfileForm::OnAliasEditMouseEnter(ui::EventArgs * args)
{
	if(!alias_edit->IsFocused())
		alias_edit->SetBorderSize(1);
	return false;
}

bool ProfileForm::OnAliasEditMouseLeave(ui::EventArgs * args)
{
	if(!alias_edit->IsFocused())
		alias_edit->SetBorderSize(0);
	return false;
}

bool nim_comp::ProfileForm::OnReturnOnAliasEdit(ui::EventArgs * args)
{
	alias_edit->GetParent()->SetFocus();
	return false;
}

void ProfileForm::InitBirthdayCombo()
{
	birth_year_combo->RemoveAll(); 
	birth_month_combo->RemoveAll();
	birth_day_combo->RemoveAll();

	auto create_elem = [](int i) 
	{
		ui::ListContainerElement* elem = new ui::ListContainerElement;
		elem->SetClass(L"listitem");
		elem->SetFixedHeight(30);
		elem->SetTextPadding(ui::UiRect(10, 1, 1, 1));
		elem->SetText(nbase::IntToString16(i));
		elem->SetFont(2);
		elem->SetStateTextColor(ui::kControlStateNormal, L"profile_account");
		return elem;
	};

	int this_year = nbase::Time::Now().ToTimeStruct(true).year();
	for (int i = this_year - 1; i >= 1900; i--)
		birth_year_combo->Add(create_elem(i));
	for (int i = 1; i <= 12; i++)
		birth_month_combo->Add(create_elem(i));
	for (int i = 1; i <= 31; i++)
		birth_day_combo->Add(create_elem(i));
}

bool ProfileForm::OnBirthdayComboSelect(ui::EventArgs * args)
{
	ui::ListContainerElement* cur_year = (ui::ListContainerElement*)birth_year_combo->GetItemAt(birth_year_combo->GetCurSel());
	ui::ListContainerElement* cur_month = (ui::ListContainerElement*)birth_month_combo->GetItemAt(birth_month_combo->GetCurSel());
	if (cur_year == NULL || cur_month == NULL)
		return false;

	int year = std::stoi(cur_year->GetText());
	int month = std::stoi(cur_month->GetText());

	std::set<int> big_months({1, 3, 5, 7, 8, 10, 12});
	std::set<int> small_months({ 4, 6, 9, 11});
	if (big_months.find(month) != big_months.cend()) //大月
	{
		for (int i = 28; i < 31; i++)
		{
			auto item = birth_day_combo->GetItemAt(i);
			if(item) item->SetVisible(true);
		}
	}
	else if (small_months.find(month) != small_months.cend()) //小月
	{
		for (int i = 28; i < 30; i++)
		{
			auto item = birth_day_combo->GetItemAt(i);
			if (item) item->SetVisible(true);
		}
		auto item = birth_day_combo->GetItemAt(30);
		if (item) 
			item->SetVisible(false);

		if (birth_day_combo->GetCurSel() > 29)
			birth_day_combo->SelectItem(29);
	}
	else //二月
	{
		for (int i = 29; i < 31; i++)
		{
			auto item = birth_day_combo->GetItemAt(i);
			if (item) item->SetVisible(false);
		}
		
		auto item = birth_day_combo->GetItemAt(28);
		if (item)
		{
			bool bissextile = (year % 4 == 0 && year % 100 != 0 || year % 400 == 0);
			birth_day_combo->GetItemAt(28)->SetVisible(bissextile);

			if (birth_day_combo->GetCurSel() > 27 + bissextile)
				birth_day_combo->SelectItem(27 + bissextile);
		}
	}

	return true;
}

void ProfileForm::OnFriendListChange(FriendChangeType change_type, const std::string& accid)
{
	if (accid == m_uinfo.GetAccId())
	{
		if (change_type == kChangeTypeAdd)
			user_type = nim::kNIMFriendFlagNormal;
		else if (change_type == kChangeTypeDelete)
			user_type = nim::kNIMFriendFlagNotFriend;
		
		add_or_del->SelectItem(user_type == nim::kNIMFriendFlagNormal ? 0 : 1);
		SetShowName();
	}
}

void ProfileForm::OnRobotChange(nim::NIMResCode rescode, nim::NIMRobotInfoChangeType type, const nim::RobotInfos& robots)
{
	if (rescode == nim::kNIMResSuccess)
	{
		for (auto &robot : robots)
		{
			if (m_robot.GetAccid() == robot.GetAccid())
			{
				InitRobotInfo(robot);
			}
		}
	}
}

void ProfileForm::OnUserInfoChange(const std::list<nim::UserNameCard>& uinfos)
{
	for (auto info : uinfos)
	{
		if (m_uinfo.GetAccId() == info.GetAccId())
		{
			m_uinfo.Update(info);
			if (info.ExistValue(nim::kUserNameCardKeyIconUrl))
				head_image_btn->SetBkImage(PhotoService::GetInstance()->GetUserPhoto(m_uinfo.GetAccId()));
			if(info.ExistValue(nim::kUserNameCardKeyName))
			{
				SetShowName();
				OnModifyOrCancelBtnClicked(nullptr, false);
			}
			break;
		}
	}
}

void nim_comp::ProfileForm::OnUserPhotoReady(PhotoType type, const std::string & account, const std::wstring & photo_path)
{
	if (type == kUser && m_uinfo.GetAccId() == account)
		head_image_btn->SetBkImage(photo_path);
}

void nim_comp::ProfileForm::OnMiscUInfoChange(const std::list<nim::UserNameCard>& uinfos)
{
	for (auto info : uinfos)
	{
		if (m_uinfo.GetAccId() == info.GetAccId())
		{
			m_uinfo.Update(info);
			InitLabels();
			OnModifyOrCancelBtnClicked(nullptr, false);
			break;
		}
	}
}

void ProfileForm::OnMultiportPushConfigChange(bool switch_on)
{
	if (-1 != user_type)
		return;

	multi_push_switch->Selected(switch_on, false);
}

void ProfileForm::InitLabels()
{
	SetShowName();

	head_image_btn->SetBkImage(PhotoService::GetInstance()->GetUserPhoto(m_uinfo.GetAccId())); //头像

	if (m_uinfo.ExistValue(nim::kUserNameCardKeyGender))
	{
		switch (m_uinfo.GetGender()) // 昵称右边的性别图标
		{
		case UG_MALE:
			sex_icon->Selected(false);
			sex_icon->SetVisible(true);
			break;
		case UG_FEMALE:
			sex_icon->Selected(true);
			sex_icon->SetVisible(true);
			break;
		case UG_UNKNOWN:
		default:
			sex_icon->SetVisible(false);
		}
	}

	std::wstring account = nbase::StringPrintf(ui::MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_PROFILE_FORM_ACCOUNT_").c_str(), nbase::UTF8ToUTF16(m_uinfo.GetAccId()).c_str());
	user_id_label->SetText(account);//帐号

	if (m_uinfo.ExistValue(nim::kUserNameCardKeyBirthday))
		birthday_label->SetText(nbase::UTF8ToUTF16(m_uinfo.GetBirth())); //生日
	if (m_uinfo.ExistValue(nim::kUserNameCardKeyMobile))
		phone_label->SetText(nbase::UTF8ToUTF16(m_uinfo.GetMobile())); //手机
	if (m_uinfo.ExistValue(nim::kUserNameCardKeyEmail))
		email_label->SetText(nbase::UTF8ToUTF16(m_uinfo.GetEmail())); //邮箱
	if (m_uinfo.ExistValue(nim::kUserNameCardKeySignature))
		signature_label->SetText(nbase::UTF8ToUTF16(m_uinfo.GetSignature())); //签名
}

void ProfileForm::SetShowName()
{
	UserService* user_service = UserService::GetInstance();
	std::wstring show_name = user_service->GetUserName(m_uinfo.GetAccId());
	show_name_label->SetText(show_name);
	ui::MutiLanSupport* mls = ui::MutiLanSupport::GetInstance();
	std::wstring title = nbase::StringPrintf(mls->GetStringViaID(L"STRID_PROFILE_FORM_WHOSE_NAMECARD").c_str(), show_name.c_str());
	SetTaskbarTitle(title); //任务栏标题

	if (user_type == nim::kNIMFriendFlagNormal) //好友
	{
		std::wstring alias = user_service->GetFriendAlias(m_uinfo.GetAccId());

		alias_box->SetVisible(true); //可以设置备注名
		if(!alias_edit->IsFocused())
			alias_edit->SetText(alias); //备注名编辑框显示备注名

		if (!alias.empty())
		{
			nickname_label->SetVisible(true); //账号下面显示昵称
			std::wstring nickname = nbase::StringPrintf(mls->GetStringViaID(L"STRID_PROFILE_FORM_NICKNAME_").c_str(), user_service->GetUserName(m_uinfo.GetAccId(), false).c_str());
			nickname_label->SetText(nickname);
		}
		else
			nickname_label->SetVisible(false);
	}
	else //不是好友，不显示备注名
	{
		nickname_label->SetVisible(false);
		alias_box->SetVisible(false);
	}
}

void ProfileForm::InitEdits()
{
	if (m_uinfo.ExistValue(nim::kUserNameCardKeyName))
		nickname_edit->SetText(nbase::UTF8ToUTF16(m_uinfo.GetName())); //昵称
	if (sex_icon->IsVisible() && m_uinfo.ExistValue(nim::kUserNameCardKeyGender))//性别
		sex_option->SelectItem(sex_icon->IsSelected());

	if (m_uinfo.ExistValue(nim::kUserNameCardKeyBirthday))
	{
		int birth[3] = { 0,0,0 }; //生日
		size_t pos = std::string::npos;
		int count = 0;
		do
		{
			size_t temp = pos + 1;
			pos = m_uinfo.GetBirth().find_first_of('-', temp);
			std::string num_str = m_uinfo.GetBirth().substr(temp, pos - temp);
			if (!num_str.empty())
				birth[count] = std::stoi(num_str);
			count++;
		} while (pos != std::string::npos && count < 3);

		int this_year = nbase::Time::Now().ToTimeStruct(true).year();
		if (birth[0] >= 1900 && birth[0] < this_year && birth_year_combo->GetItemAt(this_year - birth[0] - 1) != NULL)
			birth_year_combo->SelectItem(this_year - birth[0] - 1);
		if (birth[1] >= 1 && birth[1] <= 12 && birth_month_combo->GetItemAt(birth[1] - 1) != NULL)
			birth_month_combo->SelectItem(birth[1] - 1);
		if (birth[2] >= 1 && birth[2] <= 31 && birth_day_combo->GetItemAt(birth[2] - 1) != NULL)
			birth_day_combo->SelectItem(birth[2] - 1);

		OnBirthdayComboSelect(NULL);
	}

	if (m_uinfo.ExistValue(nim::kUserNameCardKeyMobile))
		phone_edit->SetText(nbase::UTF8ToUTF16(m_uinfo.GetMobile()));//手机
	if (m_uinfo.ExistValue(nim::kUserNameCardKeyEmail))
		email_edit->SetText(nbase::UTF8ToUTF16(m_uinfo.GetEmail()));//邮箱
	if (m_uinfo.ExistValue(nim::kUserNameCardKeySignature))
		signature_edit->SetText(nbase::UTF8ToUTF16(m_uinfo.GetSignature()));//签名
}

}