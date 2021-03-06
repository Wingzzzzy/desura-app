/*
Copyright (C) 2011 Mark Chandler (Desura Net Pty Ltd)
Copyright (C) 2014 Bad Juju Games, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA.

Contact us at legal@badjuju.com.

*/

#include "Common.h"
#include "UninstallInfoPage.h"

#include "ItemForm.h"

using namespace UI::Forms::ItemFormPage;


namespace UI
{
	namespace Forms
	{
		namespace ItemFormPage
		{
			class CloseWatcher
			{
			public:
				bool m_bIsClosed = false;
				std::mutex m_Lock;
			};
		}
	}
}

UninstallInfoPage::UninstallInfoPage(wxWindow* parent)
	: BaseInstallPage(parent)
	, m_pCloseWatcher(std::make_shared<CloseWatcher>())
{
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &UninstallInfoPage::onButtonClicked, this);


	m_labInfo = new wxStaticText( this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(-1,-1), 0 );

	wxFont font = m_labInfo->GetFont();
	font.SetPointSize(10);
	m_labInfo->SetFont(font);

#ifdef NIX
	m_labInfo->Wrap( 385 );
#else
	m_labInfo->Wrap( 360 );
#endif

	m_cbComplete = new gcCheckBox( this, wxID_ANY, Managers::GetString(L"#UNF_REMOVEFILES"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cbComplete->SetToolTip( Managers::GetString(L"#UNF_REMOVEFILES_TOOLTIP") );
	m_cbComplete->SetValue(true);

	m_cbAccount = new gcCheckBox( this, wxID_ANY, Managers::GetString(L"#UNF_REMOVEACCOUNT"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cbAccount->SetToolTip( Managers::GetString(L"#UNF_REMOVEACCOUNT_TOOLTIP") );
	m_cbAccount->SetValue(true);

	m_butCancel = new gcButton( this, wxID_ANY, Managers::GetString(L"#CANCEL"), wxDefaultPosition, wxDefaultSize, 0 );
	m_butUninstall = new gcButton( this, wxID_ANY, Managers::GetString(L"#UNINSTALL"), wxDefaultPosition, wxDefaultSize, 0 );
	m_butUninstall->SetDefault();

	wxBoxSizer* bSizer4 = new wxBoxSizer( wxHORIZONTAL );
	bSizer4->Add( 0, 0, 1, wxEXPAND, 5 );
	bSizer4->Add( m_butUninstall, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	bSizer4->Add( m_butCancel, 0, wxALL, 5 );

	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 5, 1, 0, 0 );
	fgSizer2->AddGrowableCol( 0 );
	fgSizer2->AddGrowableRow( 0 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer2->Add( m_labInfo, 0, wxTOP|wxLEFT|wxRIGHT|wxEXPAND, 5 );
	fgSizer2->Add( m_cbComplete, 0, wxTOP|wxLEFT|wxRIGHT, 5 );
	fgSizer2->Add( m_cbAccount, 0, wxTOP|wxLEFT|wxRIGHT, 5 );
	fgSizer2->Add( 0, 5, 1, wxEXPAND, 5 );
	fgSizer2->Add( bSizer4, 1, wxEXPAND, 5 );

	this->SetSizer( fgSizer2 );
	this->Layout();
	this->setParentSize(-1, 200);
}

UninstallInfoPage::~UninstallInfoPage()
{
	std::lock_guard<std::mutex> guard(m_pCloseWatcher->m_Lock);
	m_pCloseWatcher->m_bIsClosed = true;
}

void UninstallInfoPage::init()
{
	auto info = getItemInfo();

	if (!info)
	{
		Close();
		return;
	}


	bool hasPaidBranch = false;

	for (size_t x=0; x<info->getBranchCount(); x++)
	{
		auto b = info->getBranch(x);

		if (b && HasAnyFlags(b->getFlags(), UserCore::Item::BranchInfoI::BF_ONACCOUNT) && !HasAnyFlags(b->getFlags(), UserCore::Item::BranchInfoI::BF_DEMO|UserCore::Item::BranchInfoI::BF_FREE|UserCore::Item::BranchInfoI::BF_TEST))
			hasPaidBranch = true;
	}

	if (hasPaidBranch)
	{
		m_cbAccount->Enable(false);
		m_cbAccount->SetValue(false);
	}

	if (info->getId().getType() == DesuraId::TYPE_LINK)
	{
		m_cbAccount->Enable(false);
		m_cbAccount->SetValue(true);

		m_cbComplete->Enable(false);
		m_cbComplete->SetValue(true);
	}


	m_labInfo->SetLabel(gcWString(Managers::GetString(L"#UNF_CONFIRM"), info->getName()));
	m_labInfo->Wrap( 360 );
}

void UninstallInfoPage::onButtonClicked( wxCommandEvent& event )
{
	ItemForm* itemForm = dynamic_cast<ItemForm*>(GetParent());

	if (!itemForm)
	{
		GetParent()->Close();
		return;
	}

	if (event.GetId() == m_butCancel->GetId())
	{
		if (!itemForm->restorePage())
			itemForm->Close();
	}
	else if (event.GetId() == m_butUninstall->GetId())
	{
		auto closeWatcher = m_pCloseWatcher;
		bool res = itemForm->startUninstall(m_cbComplete->GetValue(), m_cbAccount->GetValue());

		{
			std::lock_guard<std::mutex> guard(closeWatcher->m_Lock);

			if (closeWatcher->m_bIsClosed)
				return;

			if (res)
			{
				m_cbComplete->Enable(false);
				m_cbAccount->Enable(false);
				m_butCancel->Enable(false);
				m_butUninstall->Enable(false);
				return;
			}
		}

		//Cant hold lock above as we will recurse onto it
		GetParent()->Close();
	}
}
