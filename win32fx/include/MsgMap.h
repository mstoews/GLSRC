//
// MsgMap.h - message maps for what the virtual handlers don't cover (spec 3.3).
//
//     class MyDialog : public win32fx::Dialog {
//         BEGIN_MSG_MAP(MyDialog)
//             ON_MSG(WM_NOTIFY, OnNotify)
//             ON_MSG_RANGE(WM_CTLCOLORMSGBOX, WM_CTLCOLORSTATIC, OnCtlColor)
//         END_MSG_MAP()
//         LRESULT OnNotify(WPARAM, LPARAM);
//         LRESULT OnCtlColor(WPARAM, LPARAM);
//     };
//
// The map is checked before the virtual handlers. Each handler returns the
// message result; for a Dialog it becomes the DlgProc result the way
// Dialog::OnMessage results do (see Dialog.h). The class argument is only
// documentation.
//
#pragma once

#define BEGIN_MSG_MAP(cls)                                                                 \
    bool ProcessMessageMap(UINT fxMsg_, WPARAM fxWParam_, LPARAM fxLParam_, LRESULT& fxResult_) override \
    {                                                                                      \
        (void)fxWParam_;                                                                   \
        (void)fxLParam_;                                                                   \
        (void)fxResult_;

#define ON_MSG(msg, fn)                                                                    \
        if (fxMsg_ == (msg)) {                                                             \
            fxResult_ = fn(fxWParam_, fxLParam_);                                          \
            return true;                                                                   \
        }

#define ON_MSG_RANGE(first, last, fn)                                                      \
        if (fxMsg_ >= (first) && fxMsg_ <= (last)) {                                       \
            fxResult_ = fn(fxWParam_, fxLParam_);                                          \
            return true;                                                                   \
        }

#define END_MSG_MAP()                                                                      \
        return false;                                                                      \
    }
