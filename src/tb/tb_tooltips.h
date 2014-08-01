#ifndef TB_TOOLTIPS_H
#define TB_TOOLTIPS_H

#include "tb_widgets_listener.h"
#include "tb_popup_window.h"
#include "tb_editfield.h"

namespace tb {

/** TBTooltipWindow implements functional  of tooltip popup, based on TBPopupWindow and contains TBEditField as content viewer*/
class TBTooltipWindow : public TBPopupWindow
{
public:
   TBOBJECT_SUBCLASS(TBTooltipWindow, TBPopupWindow);

   TBTooltipWindow(TBWidget* target);
   virtual ~TBTooltipWindow();

   bool Show(int mouse_x, int mouse_y);

   TBPoint  GetOffsetPoint() const { return TBPoint(m_offset_x, m_offset_y); }

private:
   int m_offset_x;
   int m_offset_y;

   TBRect GetAlignedRect(int x, int y);

   TBEditField m_content;
};

/** TBTooltipManager implements logic for show/hiode tooltips*/
class TBTooltipManager : private TBWidgetListener, public TBMessageHandler
{
public:
   TBTooltipManager();
   virtual ~TBTooltipManager();

   static unsigned int tooltip_point_offset_y; ///< offset by Y of tooltip point
   static unsigned int tooltip_show_delay_ms; ///< delay in ms before tooltip will be shown
   static unsigned int tooltip_show_duration_ms; ///< tooltip display duration 
   static unsigned int tooltip_hide_point_dist; /// distance by X or Y which used to hide tooltip

private:

   virtual bool OnWidgetInvokeEvent(TBWidget *widget, const TBWidgetEvent &ev);
   virtual void OnMessageReceived(TBMessage *msg);
   void KillToolTip();

   void  DeleteShowMessages();
   TBWidget*   GetTippedWidget();

   TBTooltipWindow* m_tooltip;
   TBWidget* m_last_tipped_widget;
};

extern TBTooltipManager* g_tooltip_mng;

}

#endif