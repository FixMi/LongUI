﻿// ctrl
#include <luiconf.h>
#include <control/ui_textbox.h>
// ui
#include <core/ui_window.h>
#include <core/ui_manager.h>
#include <text/ui_ctl_impl.h>
#include <input/ui_kminput.h>
#include <util/ui_ctordtor.h>
#include <util/ui_clipboard.h>
#include <core/ui_color_list.h>
#include <graphics/ui_graphics_impl.h>

// RichED
#include <../RichED/ed_txtdoc.h>
#include <../RichED/ed_txtcell.h>

// C++
#include <type_traits>



// uinamespace
namespace LongUI {
    // private text box
    struct UITextBox::Private {
        // doc type
        //using doc_t = TextBC::CBCTextDocument;
        using doc_t = RichED::CEDTextDocument;
        // ctor
        Private() noexcept;
        // dtor
        ~Private() noexcept { if (created) this->release_doc(); }
        // create_device cached bitmap
        auto create_cached_bitmap() noexcept->Result;
        // release cached bitmap
        void release_cached_bitmap() noexcept;
        // get doc object
        auto&document() noexcept { return reinterpret_cast<doc_t&>(docbuf); }
        // create doc
        void create_doc(UITextBox& box) noexcept;
        // release doc
        void release_doc() noexcept;
        // doc map to this control
        void doc_map(float point[2]) noexcept;
        // buffer type
        using aligned_st = std::aligned_storage<sizeof(doc_t), alignof(doc_t)>;
        // document() buffer
        aligned_st::type                    docbuf;
        // cluster buffer 
        POD::Vector<DWRITE_CLUSTER_METRICS> cluster_buffer;
        // text
        CUIString                           text_cached;
#ifndef NDEBUG
        // debug color
        ColorF                              dbg_color[2];
        // debug counter
        uint32_t                            dbg_counter = 0;
        // debug output
#endif
        // text changed via user-input
        bool                                text_changed = false;
        // created flag
        bool                                created = false;
        // cached
        bool                                cached = false;
    };
    /// <summary>
    /// Privates the text box.
    /// </summary>
    /// <param name="box">The box.</param>
    /// <returns></returns>
    UITextBox::Private::Private() noexcept {
        static_assert(alignof(Private) <= alignof(double), "must less than double");
#ifndef NDEBUG
        dbg_color[0] = ColorF::FromRGBA_CT<RGBA_Green>();
        dbg_color[1] = ColorF::FromRGBA_CT<RGBA_Blue>();
        dbg_color[0].a = dbg_color[1].a = 0.25f;
#endif
    }
    /// <summary>
    /// Creates the cached bitmap.
    /// </summary>
    /// <returns></returns>
    auto UITextBox::Private::create_cached_bitmap() noexcept -> Result {
        return { Result::RS_OK };
    }
    /// <summary>
    /// Releases the cached bitmap.
    /// </summary>
    /// <returns></returns>
    void UITextBox::Private::release_cached_bitmap() noexcept {
    }
    /// <summary>
    /// Releases the document().
    /// </summary>
    /// <returns></returns>
    void UITextBox::Private::release_doc() noexcept {
        detail::ctor_dtor<doc_t>::delete_obj(&docbuf);
    }
    /// <summary>
    /// Documents the map.
    /// </summary>
    /// <param name="point">The point.</param>
    /// <returns></returns>
    void UITextBox::Private::doc_map(float point[2]) noexcept {
        auto& matrix = this->document().RefMatrix();
        const auto ptr = reinterpret_cast<RichED::Point*>(point);
        *ptr = matrix.DocToScreen(*ptr);
    }
    /// <summary>
    /// Creates the document().
    /// </summary>
    /// <param name="box">The box.</param>
    /// <returns></returns>
    void UITextBox::Private::create_doc(UITextBox& box) noexcept {
        // ARG
        using namespace RichED;
        uint32_t u32color = box.m_tfBuffer.text.color.ToRGBA().primitive;
        const DocFlag flags = static_cast<DocFlag>(box.m_flag);
        RichED::DocInitArg args {
            0, Direction_L2R, Direction_T2B,
            //0, Direction_T2B, Direction_R2L,
            flags,
            box.m_chPassword, 
            box.m_uMaxLength, 0,
            VAlign_Baseline, Mode_SpaceOrCJK,
            {  box.m_tfBuffer.font.size, u32color, 0, Effect_None }
        };
        RichED::IEDTextPlatform& platform = box;
        detail::ctor_dtor<doc_t>::create(&docbuf, platform, args);
        const Result hr{ args.code };
        // XXX: 错误处理
        assert(hr);
        created = true;
    }
}


/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
auto LongUI::UITextBox::Recreate(bool release_only) noexcept -> Result {
    // --------------------- 释放数据
    // 使用了缓存?
    if (m_private->cached) {
        // 重建待使用位图
        m_private->release_cached_bitmap();
    }

    // 重建父类设备资源
    Result hr = Super::Recreate(release_only);
    // 仅仅释放?
    if (release_only) return hr;

    // --------------------- 创建

    // 使用了缓存?
    if (hr && m_private->cached) {
        // 创建待使用位图
        hr = m_private->create_cached_bitmap();
    }
    // 父类
    return hr;
}

/// <summary>
/// Ons the character input.
/// </summary>
/// <param name="ch">The char</param>
/// <returns></returns>
bool LongUI::UITextBox::private_char(char32_t ch) noexcept {
    // 暂时使用次帧刷新
    this->NextUpdate();
    assert(m_private && "OOM");
    //m_private->document().OnChar(ch);
    return m_private->document().GuiChar(ch);
}

/// <summary>
/// Creates the private.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::create_private() noexcept {
    assert(!m_private);
    m_private = new(std::nothrow) Private{};
    this->ctor_failed_if(m_private);
}

/// <summary>
/// Initializes the private.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::init_private() noexcept {
    assert(m_private && "bad action");
    // 延迟到init事件创建Doc对象
    m_private->create_doc(*this);
    // 存在初始化字符的话进行文本初始化, 当然不必输出文本changed标志
    auto& cached = m_private->text_cached;
    if (!cached.empty()) {
        this->private_set_text();
        //m_private->document().ClearTextChanged();
    }
}


/// <summary>
/// Deletes the private.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::delete_private() noexcept {
    if (m_private) delete m_private;
}


/// <summary>
/// Privates the use cached.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::private_use_cached() noexcept {
    m_private->cached = true;
}

/// <summary>
/// Privates the set text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UITextBox::private_set_text(CUIString&& text) noexcept {
    auto& cached = m_private->text_cached;
    cached = std::move(text);
}

/// <summary>
/// Privates the mark readonly.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::private_mark_readonly() noexcept {
    m_flag |= RichED::Flag_GuiReadOnly;
}


/// <summary>
/// Privates the mark multiline.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::private_mark_multiline() noexcept {
    m_flag |= RichED::Flag_MultiLine;
}

/// <summary>
/// Privates the mark password.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::private_mark_password() noexcept {
    m_flag |= RichED::Flag_UsePassword;
}



/// <summary>
/// Marks the change could trigger.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::mark_change_could_trigger() noexcept {
    assert(m_private && "BAD ACTION");
    m_private->text_changed = true;
}

/// <summary>
/// Clears the change could trigger.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::clear_change_could_trigger() noexcept {
    assert(m_private && "BAD ACTION");
    m_private->text_changed = false;
}

/// <summary>
/// Determines whether [is change could trigger].
/// </summary>
/// <returns></returns>
bool LongUI::UITextBox::is_change_could_trigger() const noexcept {
    assert(m_private && "BAD ACTION");
    m_private->text_changed = false;
    return m_private->text_changed;
}

/// <summary>
/// Shows the caret.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::show_caret() noexcept {
    assert(m_private);
    auto caret = m_private->document().GetCaret();
    // 调整到内容区域
    const auto lt = this->GetBox().GetContentPos();
    caret.x += lt.x; caret.y += lt.y;
    // GetCaret返回的矩形宽度没有意义, 可以进行自定义
    const float custom_width = 2.f;
    const float offset_rate = 0.0f;
    RectF rect = {
        caret.x - custom_width * offset_rate,       // 向前后偏移
        caret.y + 1,                                // 上下保留一个单位的空白以保持美观
        caret.x + custom_width * (1 - offset_rate), // 向后后偏移
        caret.y + caret.height - 1                  // 上下保留一个单位的空白以保持美观
    };

    m_private->doc_map(&rect.left);
    m_private->doc_map(&rect.right);
    m_pWindow->ShowCaret(*this, rect);
}

/// <summary>
/// Privates the update.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::private_update() noexcept {
    auto& doc = m_private->document();
    const auto values = doc.Update();
    // 所有修改需要重绘
    if (values) {
        this->Invalidate();
        // 选择修改
        //if (values & RichED::Changed_Selection) {
        //    LUIDebug(Hint) << "Changed_Selection" << LongUI::endl;
        //}
        // 插入符号
        if (values & RichED::Changed_Caret) {
            this->show_caret();
        }
        // 文本修改
        if (values & RichED::Changed_Text) {
            m_private->text_changed = true;
        }
    }
}


/// <summary>
/// Privates the resize.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
void LongUI::UITextBox::private_resize(Size2F size) noexcept {
    m_private->document().Resize({ size.width, size.height });
}



/// <summary>
/// Privates the mouse down.
/// </summary>
/// <param name="pos">The position.</param>
/// <param name="shift">if set to <c>true</c> [shift].</param>
/// <returns></returns>
bool LongUI::UITextBox::private_mouse_down(Point2F pos, bool shift) noexcept {
    this->NeedUpdate();
    auto& doc = m_private->document();
    const auto lt = this->GetBox().GetContentPos();
    //doc.OnLButtonDown({ pos.x - lt.x, pos.y - lt.y }, shift);
    return doc.GuiLButtonDown({ pos.x - lt.x, pos.y - lt.y }, shift);
}

/// <summary>
/// Privates the mouse up.
/// </summary>
/// <param name="pos">The position.</param>
/// <returns></returns>
bool LongUI::UITextBox::private_mouse_up(Point2F pos) noexcept {
    //auto& doc = m_private->document();
    //const auto lt = this->GetBox().GetContentPos();
    //doc.OnLButtonUp({ pos.x - lt.x, pos.y - lt.y });
    //return doc.GuiLButtonHold
    return true;
}


/// <summary>
/// Privates the mouse move.
/// </summary>
/// <param name="pos">The position.</param>
/// <returns></returns>
bool LongUI::UITextBox::private_mouse_move(Point2F pos) noexcept {
    this->NeedUpdate();
    auto& doc = m_private->document();
    const auto lt = this->GetBox().GetContentPos();
    //doc.OnLButtonHold({ pos.x - lt.x, pos.y - lt.y });
    return doc.GuiLButtonHold({ pos.x - lt.x, pos.y - lt.y });
}



#ifdef LUI_TEXTBOX_USE_UNIFIED_INPUT
/// <summary>
/// Privates the left.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::private_left() noexcept {
    const auto ctrl = CUIInputKM::GetKeyState(CUIInputKM::KB_CONTROL);
    const auto shift = CUIInputKM::GetKeyState(CUIInputKM::KB_SHIFT);
    m_private->document().OnLeft(ctrl, shift);
}

/// <summary>
/// Privates the right.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::private_right() noexcept {
    const auto ctrl = CUIInputKM::GetKeyState(CUIInputKM::KB_CONTROL);
    const auto shift = CUIInputKM::GetKeyState(CUIInputKM::KB_SHIFT);
    m_private->document().OnRight(ctrl, shift);
}

/// <summary>
/// Privates up.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::private_up() noexcept {
    const auto shift = CUIInputKM::GetKeyState(CUIInputKM::KB_SHIFT);
    m_private->document().OnUp(shift);
}

/// <summary>
/// Privates down.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::private_down() noexcept {
    const auto shift = CUIInputKM::GetKeyState(CUIInputKM::KB_SHIFT);
    m_private->document().OnDown(shift);
}
#endif
/// <summary>
/// Privates the keydown.
/// </summary>
/// <param name="key">The key.</param>
/// <returns></returns>
bool LongUI::UITextBox::private_keydown(uint32_t key) noexcept {
    bool error_code = true;
    this->NeedUpdate();
    auto& doc = m_private->document();
    const auto ctrl = CUIInputKM::GetKeyState(CUIInputKM::KB_CONTROL);
    const auto shift = CUIInputKM::GetKeyState(CUIInputKM::KB_SHIFT);
    switch (static_cast<CUIInputKM::KB>(key))
    {
    case CUIInputKM::KB_BACK:
        // 退格键
        error_code = doc.GuiBackspace(ctrl);
        break;
    case CUIInputKM::KB_DELETE:
        // 删除键
        error_code = doc.GuiDelete(ctrl);
        break;
    case CUIInputKM::KB_LEFT:
        // 左箭头键
        error_code = doc.GuiLeft(ctrl, shift);
        break;
    case CUIInputKM::KB_RIGHT:
        // 右箭头键
        error_code = doc.GuiRight(ctrl, shift);
        break;
    case CUIInputKM::KB_UP:
        // 上箭头键
        error_code = doc.GuiUp(ctrl, shift);
        break;
    case CUIInputKM::KB_DOWN:
        // 下箭头键
        error_code = doc.GuiDown(ctrl, shift);
        break;
    case CUIInputKM::KB_RETURN:
        // 回车返回键
        if (!doc.GuiReturn()) {
            // 处理
            const auto c = this->try_trigger_change_event();
            const auto i = Super::TriggerEvent(this->_onInput()) != Event_Ignore;
            error_code = c || i;
        }
        break;
    case CUIInputKM::KB_A:
        // A 键
        if (ctrl) error_code = doc.GuiSelectAll();
        break;
    case CUIInputKM::KB_Z:
        if (ctrl) error_code = doc.GuiUndo();
        break;
    case CUIInputKM::KB_Y:
        if (ctrl) error_code = doc.GuiRedo();
        break;
    case CUIInputKM::KB_X:
    case CUIInputKM::KB_C:
        // X, C 键
        if (ctrl) {
            // 获取选中文本
            //CUIString text;
            //doc.RequestSelected(text);
            //// 无选择或其他原因
            //if (text.empty()) break;
            //LongUI::CopyTextToClipboard(text.view());
            //// ctrl-x 是剪切
            //if (key == CUIInputKM::KB_X)
            //    doc.DeleteSelection();
        }
        break;
    case CUIInputKM::KB_V:
        // V 键
        if (ctrl) {
            CUIString text;
            LongUI::PasteTextToClipboard(text);
            static_assert(sizeof(*text.data()) == sizeof(char16_t), "");
            const auto ptr = reinterpret_cast<const char16_t*>(text.c_str());
            error_code = doc.GuiText({ ptr,  ptr + text.length() });
        }
        break;
    }
    return error_code;
}

/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::Render() const noexcept {
    Super::Render();
#ifndef NDEBUG
    m_private->dbg_counter = 0;
#endif // !NDEBUG
    auto& ctx = UIManager.Ref2DRenderer();
    // 设置渲染偏移
    const auto lt = this->GetBox().GetContentPos();
    Matrix3X2F matrix;
    ctx.GetTransform(&auto_cast(matrix));
    matrix._31 += lt.x;
    matrix._32 += lt.y;
    ctx.SetTransform(&auto_cast(matrix));
    // 设置剪切区域
    const auto sz = this->GetBox().GetContentSize();
    const auto mode = D2D1_ANTIALIAS_MODE_PER_PRIMITIVE;
    ctx.PushAxisAlignedClip({ 0, 0, sz.width, sz.height }, mode);
    // TODO: 是移动到渲染线程加UI锁?
    //       还是移动到UI线程加渲染锁?
    //       还是用一个自己的锁?
    {
        CUIDataAutoLocker locker;
        this->draw_selection(ctx);
        m_private->document().Render(&ctx);
    }
    // 弹出剪切区域
    ctx.PopAxisAlignedClip();
}


/// <summary>
/// Sets the text.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::private_set_text() noexcept {
    const auto text = m_private->text_cached.view();
    //m_private->document().SetText({ text.begin(), text.end() });
    // 删除全部再添加 XXX: ???
    m_private->document().RemoveText({ 0, 0 }, { uint32_t(-1), 0 });
    m_private->document().InsertText({ 0, 0 }, { text.begin(), text.end() });
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UITextBox::SetText(CUIString&& text) noexcept {
    //this->SetText(text.view());
    auto& cached = m_private->text_cached;
    cached = std::move(text);
    m_bTextChanged = true;
    this->NeedUpdate();
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="view">The view.</param>
/// <returns></returns>
void LongUI::UITextBox::SetText(U16View view) noexcept {
    this->SetText(CUIString(view));
}


// --------------------- IEDTextPlatform 实现 -------------------------



/// <summary>
/// Called when [oom].
/// </summary>
/// <param name="retry_count">The retry count.</param>
/// <param name="size">The size.</param>
/// <returns></returns>
auto LongUI::UITextBox::OnOOM(uint32_t retry_count, size_t size) noexcept->RichED::HandleOOM {
    const auto code = UIManager.HandleOOM(retry_count, size);
    return static_cast<RichED::HandleOOM>(code);
}

/// <summary>
/// Determines whether [is valid password] [the specified ch].
/// </summary>
/// <param name="ch">The ch.</param>
/// <returns></returns>
bool LongUI::UITextBox::IsValidPassword(char32_t ch) noexcept {
    return ch > 0x20 && ch < 0x80;
}

/// <summary>
/// Appends the text.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <param name="view">The view.</param>
/// <returns></returns>
bool LongUI::UITextBox::AppendText(CtxPtr ctx, RichED::U16View view) noexcept {
    const auto string = static_cast<CUIString*>(ctx);
    const auto guiview = U16View{ view.first, view.second };
    string->append(guiview);
    return string->is_ok();
}

/// <summary>
/// Writes to file.
/// </summary>
/// <param name="">The .</param>
/// <param name="data">The data.</param>
/// <param name="len">The length.</param>
/// <returns></returns>
bool LongUI::UITextBox::WriteToFile(CtxPtr, const uint8_t data[], uint32_t len) noexcept {
    return false;
}

/// <summary>
/// Reads from file.
/// </summary>
/// <param name="">The .</param>
/// <param name="data">The data.</param>
/// <param name="len">The length.</param>
/// <returns></returns>
bool LongUI::UITextBox::ReadFromFile(CtxPtr, uint8_t data[], uint32_t len) noexcept {
    return false;
}


/// <summary>
/// Recreates the context.
/// </summary>
/// <param name="cell">The cell.</param>
/// <returns></returns>
void LongUI::UITextBox::RecreateContext(CEDTextCell& cell) noexcept {
    // 图片
    if (cell.RefMetaInfo().metatype == RichED::Type_Image)
        this->recreate_img_context(cell);
    // 正常文本
    else
        this->recreate_nom_context(cell);
}

/// <summary>
/// Deletes the context.
/// </summary>
/// <param name="cell">The cell.</param>
/// <returns></returns>
void LongUI::UITextBox::DeleteContext(CEDTextCell& cell) noexcept {
#ifndef NDEBUG
    if (cell.ctx.context) {
        int bk = 9;
    }
#endif // !NDEBUG
    auto& ptr = reinterpret_cast<IUnknown*&>(cell.ctx.context);
    // 图片
    if (cell.RefMetaInfo().metatype == RichED::Type_Image) 
        LongUI::SafeRelease(reinterpret_cast<I::Bitmap*&>(cell.ctx.context));
    // 其他
    else
        LongUI::SafeRelease(reinterpret_cast<I::Text*&>(cell.ctx.context));
}

/// <summary>
/// Draws the context.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <param name="cell">The cell.</param>
/// <param name="baseline">The baseline.</param>
/// <returns></returns>
void LongUI::UITextBox::DrawContext(CtxPtr ctx, CEDTextCell& cell, unit_t baseline) noexcept {
    // 图片
    if (cell.RefMetaInfo().metatype == RichED::Type_Image)
        this->draw_img_context(ctx, cell, baseline);
    // 文本
    else
        this->draw_nom_context(ctx, cell, baseline);
    // 效果
    this->draw_efx_context(ctx, cell, baseline);
}




/// <summary>
/// Hits the test.
/// </summary>
/// <param name="cell">The cell.</param>
/// <param name="offset">The offset.</param>
/// <returns></returns>
auto LongUI::UITextBox::HitTest(CEDTextCell& cell, unit_t offset) noexcept -> RichED::CellHitTest {
    RichED::CellHitTest rv = { 0 };
    // 内联对象
    if (cell.RefMetaInfo().metatype >= RichED::Type_InlineObject) {
        // 内联对象长度为1
        rv.length = 1;
        // 超过一半算后面
        if (offset >= cell.metrics.width * 0.5) rv.pos = 1;
    }
    // 文本对象
    else {
        // 睡眠状态则唤醒
        if (!cell.ctx.context) this->RecreateContext(cell);
        // 失败则不获取
        if (const auto ptr = static_cast<I::Text*>(cell.ctx.context)) {
            //offset -= cell.metrics.offset.x;
            BOOL hint = false, inside = false;
            DWRITE_HIT_TEST_METRICS htm;
            ptr->HitTestPoint(offset, offset, &hint, &inside, &htm);
            rv.pos = htm.textPosition;
            rv.trailing = hint;
            rv.length = htm.length;
            // 密码帮助
            m_private->document().PWHelperHit(cell, rv);
        }
    }
    return rv;
}


/// <summary>
/// Gets the character metrics.
/// </summary>
/// <param name="cell">The cell.</param>
/// <param name="pos">The position.</param>
/// <returns></returns>
auto LongUI::UITextBox::GetCharMetrics(CEDTextCell& cell, uint32_t pos) noexcept -> RichED::CharMetrics {
    RichED::CharMetrics cm = { 0 };
    // 内联对象
    if (cell.RefMetaInfo().metatype >= RichED::Type_InlineObject) {
        // 只有前后之分
        if (pos) {
            cm.offset = cell.metrics.width;
            cm.width = 0;
        }
        else {
            cm.offset = 0;
            cm.width = cell.metrics.width;
        }
    }
    // 文本对象
    else {
        // 睡眠状态则唤醒
        if (!cell.ctx.context) this->RecreateContext(cell);
        // 失败则不获取
        if (const auto ptr = static_cast<I::Text*>(cell.ctx.context)) {
            // 末尾
            if (pos == cell.RefString().length) {
                cm.offset = cell.metrics.width;
            }
            else {
                // 密码模式
                const auto real_pos = m_private->document().PWHelperPos(cell, pos);
                assert(m_private);
                auto& buf = m_private->cluster_buffer;
                const uint32_t size = real_pos + 1;
                buf.resize(size);
                if (buf.is_ok()) {
                    uint32_t max_count = 0;
                    ptr->GetClusterMetrics(buf.data(), size, &max_count);
                    // 遍历位置
                    const auto data_ptr = buf.data();
                    const auto data_len = max_count;
                    float width = 0, last_width = 0;
                    uint32_t char_index = 0;
                    // 防止万一, 加上 [i != data_len]
                    for (uint32_t i = 0; i != data_len; ++i) {
                        const auto&x = data_ptr[i];
                        width += last_width = x.width;
                        // 防止万一用>=
                        if (char_index >= real_pos) break;
                        char_index += x.length;
                    }
                    // 写回返回值
                    cm.width = last_width;
                    cm.offset = width - last_width;
                }
            }
        }
    }
    return cm;
}



#ifndef NDEBUG
/// <summary>
/// Debugs the output.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
void LongUI::UITextBox::DebugOutput(const char* txt, bool high) noexcept {
    if (high) 
        LUIDebug(Error) << CUIString::FromUtf8(txt) << endl;
    else if (this->dbg_output) 
        LUIDebug(Hint) << CUIString::FromUtf8(txt) << endl;
}
#endif



/// <summary>
/// Draws the selection.
/// </summary>
/// <param name="renderer">The renderer.</param>
/// <returns></returns>
void LongUI::UITextBox::draw_selection(I::Renderer2D& renderer) const noexcept {
    const auto& vec = m_private->document().RefSelection();
    if (!vec.GetSize()) return;
    // XXX: 获取选择颜色
    auto& brush = UIManager.RefCCBrush(ColorF::FromRGBA_CT<RGBA_TianyiBlue>());
    for (auto& rect : vec) {
        D2D1_RECT_F rc;
        rc.left = rect.left;
        rc.top = rect.top;
        rc.right = rect.right;
        rc.bottom = rect.bottom;
        m_private->doc_map(&rc.left);
        m_private->doc_map(&rc.right);
        renderer.FillRectangle(rc, &brush);
    }
}

/// <summary>
/// Draws the img context.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <param name="cell">The cell.</param>
/// <param name="baseline">The baseline.</param>
/// <returns></returns>
void LongUI::UITextBox::draw_img_context(CtxPtr ctx, CEDTextCell& cell, unit_t baseline) const noexcept {
    assert(!"NOT IMPL");
}

/// <summary>
/// Draws the nom context.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <param name="cell">The cell.</param>
/// <param name="baseline">The baseline.</param>
/// <returns></returns>
void LongUI::UITextBox::draw_nom_context(CtxPtr ctx, CEDTextCell& cell, unit_t baseline) const noexcept {
    const auto renderer = static_cast<I::Renderer2D*>(ctx);
    D2D1_POINT_2F point; assert(renderer);
    point.x = cell.metrics.pos + cell.metrics.offset.x;
    // 失败则不渲染
    if (const auto ptr = static_cast<I::Text*>(cell.ctx.context)) {
        // 渲染CELL
        point.y = baseline - cell.metrics.ar_height + cell.metrics.offset.y;
#ifndef NDEBUG
        if (UIManager.flag & ConfigureFlag::Flag_DbgDrawTextCell) {
            const auto& color_d = m_private->dbg_color[++m_private->dbg_counter & 1];
            D2D1_RECT_F cell_rect;
            cell_rect.left = point.x + cell.metrics.bounding.left;
            cell_rect.top = point.y + cell.metrics.bounding.top;
            cell_rect.right = point.x + cell.metrics.bounding.right;
            cell_rect.bottom = point.y + cell.metrics.bounding.bottom;
            m_private->doc_map(&cell_rect.left);
            m_private->doc_map(&cell_rect.right);
            renderer->FillRectangle(&cell_rect, &UIManager.RefCCBrush(color_d));
        }
#endif
        m_private->doc_map(&point.x);
        ColorF color; ColorF::FromRGBA_RT(color, { cell.RefRichED().color });
        auto& brush = UIManager.RefCCBrush(color);
        renderer->DrawTextLayout(point, ptr, &brush);
    }

}


/// <summary>
/// Draws the efx context.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <param name="cell">The cell.</param>
/// <param name="baseline">The baseline.</param>
/// <returns></returns>
void LongUI::UITextBox::draw_efx_context(CtxPtr ctx, CEDTextCell & cell, unit_t baseline) const noexcept {
    // TODO: 富文本效果
    if (!(m_private->document().RefInfo().flags & RichED::Flag_RichText)) return;
    const auto renderer = static_cast<I::Renderer2D*>(ctx);
    D2D1_POINT_2F point; assert(renderer);
    point.x = cell.metrics.pos + cell.metrics.offset.x;
    // 下划线
    if (cell.RefRichED().effect & RichED::Effect_Underline) {
        point.y = baseline + cell.metrics.dr_height + cell.metrics.offset.y;
        auto point2 = point; point2.x += cell.metrics.width;
        m_private->doc_map(&point.x);
        m_private->doc_map(&point2.x);
        ColorF color; ColorF::FromRGBA_RT(color, { cell.RefRichED().color });
        auto& brush = UIManager.RefCCBrush(color);
        renderer->DrawLine(point, point2, &brush);
    }
}


/// <summary>
/// Recreates the img context.
/// </summary>
/// <param name="cell">The cell.</param>
/// <returns></returns>
void LongUI::UITextBox::recreate_img_context(CEDTextCell& cell) noexcept {
    assert(!"NOT IMPL");
}


/// <summary>
/// Recreates the nom context.
/// </summary>
/// <param name="cell">The cell.</param>
/// <returns></returns>
void LongUI::UITextBox::recreate_nom_context(CEDTextCell& cell) noexcept {
    // 释放之前的文本
    auto prev_text = reinterpret_cast<I::Text*>(cell.ctx.context);
    cell.ctx.context = nullptr;
    if (prev_text) prev_text->Release();
    // 利用字符串创建CTL文本布局
    const auto& str = cell.RefString();
    // 空的场合
    if (!str.length) {
        if (cell.RefMetaInfo().dirty) {
            cell.metrics.width = 0;
            const auto h = cell.RefRichED().size;
            cell.metrics.ar_height = h;
            cell.metrics.dr_height = 0;
            cell.metrics.bounding.left = 0;
            cell.metrics.bounding.top = 0;
            cell.metrics.bounding.right = 0;
            cell.metrics.bounding.bottom = h;
            cell.AsClean();
        }
        return;
    }
    // 创建对应字体
    assert(m_private);
    auto& doc = m_private->document();
    // 富文本
    //m_private->document().RefInfo.flags & RichED::Flag_RichText;
    auto& text = reinterpret_cast<I::Text*&>(cell.ctx.context);
    // 密码帮助
    Result hr = m_private->document().PWHelperView([&text](RichED::U16View view) noexcept {
        TextArg targ = {};
        // TODO: 字体/富文本
        targ.string = view.first;
        targ.length = view.second - view.first;
        return UIManager.CreateCtlText(targ, luiref text);
    }, cell);
    // 测量CELL
    if (cell.RefMetaInfo().dirty) {
        const auto layout = reinterpret_cast<IDWriteTextLayout*>(cell.ctx.context);
        const bool ver = doc.RefMatrix().read_direction & 1;
        DWRITE_TEXT_METRICS dwtm;
        DWRITE_LINE_METRICS dwlm;
        DWRITE_OVERHANG_METRICS dwom;
        enum {
            LUI_READING_TOP_TO_BOTTOM = 2,
            LUI_FLOW_RIGHT_TO_LEFT = 3,
        };
#if LUI_COMPILER == LUI_COMPILER_MSVC
        static_assert(LUI_READING_TOP_TO_BOTTOM == DWRITE_READING_DIRECTION_TOP_TO_BOTTOM, "SAME!");
        static_assert(LUI_FLOW_RIGHT_TO_LEFT == DWRITE_FLOW_DIRECTION_RIGHT_TO_LEFT, "SAME!");
#endif
        if (hr) {
            // 垂直布局
            if (ver) {
                layout->SetReadingDirection(DWRITE_READING_DIRECTION(LUI_READING_TOP_TO_BOTTOM));
                layout->SetFlowDirection(DWRITE_FLOW_DIRECTION(LUI_FLOW_RIGHT_TO_LEFT));
            }
            hr.code = layout->GetMetrics(&dwtm);
        }
        if (hr) {
            hr.code = layout->GetOverhangMetrics(&dwom);
        }
        if (hr) {
            uint32_t count = 1;
            hr.code = layout->GetLineMetrics(&dwlm, 1, &count);
        }
        if (hr) {
            // 垂直
            if (ver) {
                cell.metrics.width = dwtm.height;
                doc.VAlignHelperH(dwlm.baseline, dwlm.height, cell.metrics);
                cell.metrics.bounding.left = -dwom.top;
                cell.metrics.bounding.top = -dwom.right;
                cell.metrics.bounding.right = dwom.bottom;
                cell.metrics.bounding.bottom = dwom.left;
            }
            // 水平
            else {
                cell.metrics.width = dwtm.widthIncludingTrailingWhitespace;
                doc.VAlignHelperH(dwlm.baseline, dwlm.height, cell.metrics);
                cell.metrics.bounding.left = -dwom.left;
                cell.metrics.bounding.top = -dwom.top;
                cell.metrics.bounding.right = dwom.right;
                cell.metrics.bounding.bottom = dwom.bottom;
            }

        }
        // CLEAN!
        cell.AsClean();
    }
}