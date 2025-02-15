
/*

    Goldleaf - Multipurpose homebrew tool for Nintendo Switch
    Copyright (C) 2018-2019  XorTroll

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#include <ui/ui_PCExploreLayout.hpp>
#include <ui/ui_MainApplication.hpp>

extern ui::MainApplication::Ref global_app;
extern cfg::Settings global_settings;

namespace ui
{
    PCExploreLayout::PCExploreLayout() : pu::ui::Layout()
    {
        this->pathsMenu = pu::ui::elm::Menu::New(0, 160, 1280, global_settings.custom_scheme.Base, global_settings.menu_item_size, (560 / global_settings.menu_item_size));
        this->pathsMenu->SetOnFocusColor(global_settings.custom_scheme.BaseFocus);
        global_settings.ApplyScrollBarColor(this->pathsMenu);
        this->Add(this->pathsMenu);
    }

    void PCExploreLayout::UpdatePaths()
    {
        this->names.clear();
        this->paths.clear();
        this->pathsMenu->ClearItems();
        u32 drivecount = 0;
        u32 pathcount = 0;
        Result rc = usb::ProcessCommand<usb::CommandId::GetDriveCount>(usb::Out32(drivecount));
        if(R_SUCCEEDED(rc))
        {
            for(u32 i = 0; i < drivecount; i++)
            {
                String label;
                String path;
                u32 sztmp = 0;
                rc = usb::ProcessCommand<usb::CommandId::GetDriveInfo>(usb::In32(i), usb::OutString(label), usb::OutString(path), usb::Out32(sztmp), usb::Out32(sztmp));
                if(R_SUCCEEDED(rc))
                {
                    this->names.push_back(label + " (" + path + ":\\)");
                    this->paths.push_back(path);
                }
            }
        }
        rc = usb::ProcessCommand<usb::CommandId::GetSpecialPathCount>(usb::Out32(pathcount));
        if(R_SUCCEEDED(rc))
        {
            for(u32 i = 0; i < pathcount; i++)
            {
                String name;
                String path;
                rc = usb::ProcessCommand<usb::CommandId::GetSpecialPath>(usb::In32(i), usb::OutString(name), usb::OutString(path));
                if(R_SUCCEEDED(rc))
                {
                    this->names.push_back(name);
                    this->paths.push_back(path);    
                }
            }
        }
        for(u32 i = 0; i < this->names.size(); i++)
        {
            auto itm = pu::ui::elm::MenuItem::New(this->names[i]);
            itm->SetColor(global_settings.custom_scheme.Text);
            if(i < drivecount) itm->SetIcon(global_settings.PathForResource("/Common/Drive.png"));
            else itm->SetIcon(global_settings.PathForResource("/FileSystem/Directory.png"));
            itm->AddOnClick(std::bind(&PCExploreLayout::path_Click, this));
            this->pathsMenu->AddItem(itm);
        }
        auto fselitm = pu::ui::elm::MenuItem::New(cfg::strings::Main.GetString(403));
        fselitm->SetColor(global_settings.custom_scheme.Text);
        fselitm->SetIcon(global_settings.PathForResource("/FileSystem/File.png"));
        fselitm->AddOnClick(std::bind(&PCExploreLayout::fileSelect_Click, this));
        this->pathsMenu->AddItem(fselitm);
        this->pathsMenu->SetSelectedIndex(0);
    }

    void PCExploreLayout::path_Click()
    {
        u32 idx = this->pathsMenu->GetSelectedIndex();
        global_app->GetBrowserLayout()->ChangePartitionPCDrive(this->paths[idx]);
        global_app->LoadLayout(global_app->GetBrowserLayout());
    }

    void PCExploreLayout::fileSelect_Click()
    {
        String selfile;
        auto rc = usb::ProcessCommand<usb::CommandId::SelectFile>(usb::OutString(selfile));
        if(R_SUCCEEDED(rc)) global_app->GetBrowserLayout()->HandleFileDirectly(selfile);
    }
}