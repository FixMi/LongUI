﻿#pragma once
/**
* Copyright (c) 2014-2018 dustpg   mailto:dustpg@gmail.com
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following
* conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/

// ui header
#include "ui_control.h"
#include "../style/ui_text.h"
#include "../core/ui_string.h"
#include "../core/ui_core_type.h"
#include "../text/ui_text_layout.h"
#include "../core/ui_const_sstring.h"
#include "../util/ui_named_control.h"

// RichED
#include "../../RichED/ed_txtplat.h"
// TextBC
//#include <../TextBC/bc_txtplat.h>

// cursor
#include <graphics/ui_cursor.h>

// ui namespace
namespace LongUI {
    // longui <-> riched
    using RichED::unit_t;
    using RichED::CtxPtr;
    using RichED::CEDTextCell;
    // textbox
    class UITextBox : public UIControl, 
        protected RichED::IEDTextPlatform/*,
        protected TextBC::IBCTextPlatform*/ {
        // super class
        using Super = UIControl;
        // text content
        //using Text = TextBC::IBCTextContent;
        // private impl
        struct Private;
    protected:
        // ctor
        UITextBox(UIControl* parent, const MetaControl&) noexcept;
    public:
        // When [pressed enter key, or killed-focus] if text changed
        static constexpr auto _onChange() noexcept { return GuiEvent::Event_OnChange; }
        // This event is sent when a user enters text in a textbox
        static constexpr auto _onInput() noexcept { return GuiEvent::Event_OnInput; }
        // selection changed
        //static inline constexpr auto _selectionChanged() noexcept { return GuiEvent::Event_Select; }
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~UITextBox() noexcept;
        // ctor
        UITextBox(UIControl* parent = nullptr) noexcept :UITextBox(parent, UITextBox::s_meta) {}
    public:
        // trigger event
        auto TriggerEvent(GuiEvent event) noexcept->EventAccept override;
        // normal event
        auto DoEvent(UIControl*, const EventArg& e) noexcept->EventAccept override;
        // update
        void Update() noexcept override;
        // render
        void Render() const noexcept override;
        // mouse event
        auto DoMouseEvent(const MouseEventArg& e) noexcept -> EventAccept override;
        // input event
        auto DoInputEvent(InputEventArg e) noexcept->EventAccept override;
        // recreate device resource
        auto Recreate(bool release_only) noexcept->Result override;
    protected:
        // on out of memory, won't be called on ctor
        auto OnOOM(uint32_t retry_count, size_t size) noexcept->RichED::HandleOOM override;
        // is valid password
        bool IsValidPassword(char32_t) noexcept override;
        // append text
        bool AppendText(CtxPtr ctx, RichED::U16View view) noexcept override;
        // write to file
        bool WriteToFile(CtxPtr, const uint8_t data[], uint32_t len) noexcept override;
        // read from file
        bool ReadFromFile(CtxPtr, uint8_t data[], uint32_t len) noexcept override;
        // recreate context
        void RecreateContext(CEDTextCell& cell) noexcept final override;
        // delete context
        void DeleteContext(CEDTextCell&) noexcept final override;
        // draw context
        void DrawContext(CtxPtr,CEDTextCell&, unit_t baseline) noexcept override;
        // hit test
        auto HitTest(CEDTextCell&, unit_t offset) noexcept->RichED::CellHitTest override;
        // get char metrics
        auto GetCharMetrics(CEDTextCell&, uint32_t pos) noexcept->RichED::CharMetrics override;
#ifndef NDEBUG
        // debug output
        void DebugOutput(const char*, bool high) noexcept override;
#endif
    protected:
        // draw selection
        void draw_selection(I::Renderer2D&) const noexcept;
        // draw img context
        void draw_img_context(CtxPtr,CEDTextCell& cell, unit_t baseline) const noexcept;
        // draw normal context
        void draw_nom_context(CtxPtr,CEDTextCell& cell, unit_t baseline) const noexcept;
        // draw effect context
        void draw_efx_context(CtxPtr, CEDTextCell& cell, unit_t baseline) const noexcept;
        // recreate img context
        void recreate_img_context(CEDTextCell& cell) noexcept;
        // recreate normal context
        void recreate_nom_context(CEDTextCell& cell) noexcept;
    protected:
        // add attribute
        void add_attribute(uint32_t key, U8View value) noexcept override;
        // try trigger change event
        bool try_trigger_change_event() noexcept;
        // mark change event could be triggered
        void mark_change_could_trigger() noexcept;
        // clear change event could be triggered
        void clear_change_could_trigger() noexcept;
        // is change event could be triggered?
        bool is_change_could_trigger() const noexcept;
        // show caret
        void show_caret() noexcept;
    public:
        // set text
        void SetText(CUIString&& text) noexcept;
        // set text
        void SetText(U16View view) noexcept;
        // request text, not const method
        auto RequestText() noexcept -> const CUIString&;
    private:
        // text used font
        TextFont                m_tfBuffer;
        // hovered curor
        CUICursor               m_hovered;
        // private data
        Private*                m_private = nullptr;
        // max length
        uint32_t                m_uMaxLength = 0x00ffffff;
        // flag
        uint32_t                m_flag = 0;
        // password char
        char32_t                m_chPassword = 0x25cf;
        // cols/size
        uint32_t                m_uCols = 20;
        // rows
        uint32_t                m_uRows = 1;
        // init textbox
        void init_textbox() noexcept;
        // init private data
        void init_private() noexcept;
        // create private data
        void create_private() noexcept;
        // delete private data
        void delete_private() noexcept;
        // set text
        void private_set_text() noexcept;
        // private use cached
        void private_use_cached() noexcept;
        // private set text
        void private_set_text(CUIString&& text) noexcept;
        // private mark readonly
        void private_mark_readonly() noexcept;
        // private mark multiline
        void private_mark_multiline() noexcept;
        // private mark password
        void private_mark_password() noexcept;
        // private mouse down
        bool private_mouse_down(Point2F, bool shift) noexcept;
        // private mouse up
        bool private_mouse_up(Point2F) noexcept;
        // private mouse up
        bool private_mouse_move(Point2F) noexcept;
        // private key down
        bool private_keydown(uint32_t key) noexcept;
        // on char input
        bool private_char(char32_t) noexcept;
        // private update
        void private_update() noexcept;
        // private resized
        void private_resize(Size2F) noexcept;
#ifdef LUI_TEXTBOX_USE_UNIFIED_INPUT
        // private left
        void private_left() noexcept;
        // private up
        void private_up() noexcept;
        // private right
        void private_right() noexcept;
        // private down
        void private_down() noexcept;
#endif
    public:

    };
    // get meta info for UITextBox
    LUI_DECLARE_METAINFO(UITextBox);
}