#include <math.h>

#include "tb_widgets_listener.h"
#include "tb_tooltips.h"
#include "tb_language.h"

using namespace tb;

tb::TBTooltipManager* tb::g_tooltip_mng = nullptr;

unsigned int TBTooltipManager::tooltip_point_offset_y = 20;
unsigned int TBTooltipManager::tooltip_show_delay_ms = 700;
unsigned int TBTooltipManager::tooltip_show_duration_ms = 5000;
unsigned int TBTooltipManager::tooltip_hide_point_dist = 40;


namespace {

const TBID messageShow = TBIDC("TBTooltipManager.show");
const TBID messageHide = TBIDC("TBTooltipManager.hide");

 class TTMsgParam: public TBTypedObject
 {
 public:
    TTMsgParam(TBWidget* hovered) : m_hovered(hovered){}
    
   TBWidget* m_hovered;
 };

}


TBTooltipWindow::TBTooltipWindow (TBWidget* target)
   :TBPopupWindow(target)
{
   SetSkinBg("", WIDGET_INVOKE_INFO_NO_CALLBACKS);
   SetSettings(WINDOW_SETTINGS_NONE);
   m_content.SetSkinBg(TBIDC("TBTooltip"), WIDGET_INVOKE_INFO_NO_CALLBACKS);
   m_content.SetIsFocusable(false);
   m_content.SetStyling(true);
   m_content.SetGravity(WIDGET_GRAVITY_ALL);
   m_content.SetReadOnly(true);
   m_content.SetMultiline(true);
   m_content.SetText(target->GetDescription());
   m_content.SetAdaptToContentSize(true);
   AddChild(&m_content);
}

TBTooltipWindow::~TBTooltipWindow ()
{
   RemoveChild(&m_content);
}

bool TBTooltipWindow::Show (int mouse_x, int mouse_y)
{
   m_offset_x = mouse_x;
   m_offset_y = mouse_y;
 
   GetTargetWidget().Get()->GetParentRoot()->AddChild(this);
   SetRect(GetAlignedRect(m_offset_x, mouse_y));
   return true;
}


TBRect TBTooltipWindow::GetAlignedRect (int x, int y)
{
   TBWidget *root = GetParentRoot();

   SizeConstraints sc(root->GetRect().w, root->GetRect().h);

   PreferredSize ps = GetPreferredSize(sc);

   TBPoint pos(x, y);
   int w = MIN(ps.pref_w, root->GetRect().w);
   int h = MIN(ps.pref_h, root->GetRect().h);

   x = pos.x + w > root->GetRect().w ? pos.x - w : pos.x;
   y = pos.y;
   if (pos.y + h > root->GetRect().h)
      y = pos.y - TBTooltipManager::tooltip_point_offset_y - h;

   return TBRect(x, y, w, h);
}


//////////////////////////////////////////////////////////////////////////

TBTooltipManager::TBTooltipManager ()
   :m_tooltip(nullptr), m_last_tipped_widget(nullptr)
{
   TBWidgetListener::AddGlobalListener(this);
}

TBTooltipManager::~TBTooltipManager ()
{
   TBWidgetListener::RemoveGlobalListener(this);
}

bool TBTooltipManager::OnWidgetInvokeEvent (TBWidget *widget, const TBWidgetEvent &ev)
{
   if (ev.type == EVENT_TYPE_POINTER_MOVE && !TBWidget::captured_widget) 
   {
      TBWidget* tipped_widget = GetTippedWidget();
      if (m_last_tipped_widget != tipped_widget && tipped_widget)
      {
         TBMessageData* msg_data = new TBMessageData();
         msg_data->v1.SetObject(new TTMsgParam(tipped_widget));
         PostMessageDelayed(messageShow, msg_data, tooltip_show_delay_ms);

      }else if (m_last_tipped_widget == tipped_widget && tipped_widget && m_tooltip) 
      {
         int x = TBWidget::pointer_move_widget_x;
         int y = TBWidget::pointer_move_widget_y;
         tipped_widget->ConvertToRoot(x, y);
         y += tooltip_point_offset_y;
         TBPoint tt_point = m_tooltip->GetOffsetPoint();
         if (abs(tt_point.x - x) > (int)tooltip_hide_point_dist || abs(tt_point.y - y) > (int)tooltip_hide_point_dist) 
         {
            KillToolTip();
            DeleteShowMessages();
         }
      }else if (!tipped_widget) 
      {
         KillToolTip();
         DeleteShowMessages();
      }
      m_last_tipped_widget = tipped_widget;
   }else// if (ev.type == EVENT_TYPE_POINTER_DOWN || ev.type == EVENT_TYPE_POINTER_UP) 
   {
      KillToolTip();
      DeleteShowMessages();
   }
   
   return false;
}

void TBTooltipManager::KillToolTip ()
{
   if (m_tooltip) 
   {
      m_tooltip->Close();
      m_tooltip = nullptr;
   }
}

void TBTooltipManager::DeleteShowMessages ()
{
   TBMessage* msg;
   while((msg = GetMessageByID(messageShow)) != nullptr) 
      DeleteMessage(msg);
}

TBWidget* TBTooltipManager::GetTippedWidget ()
{
   TBWidget* current = TBWidget::hovered_widget;
   while(current && current->GetDescription().IsEmpty())
      current = current->GetParent();
   return current;
}

void TBTooltipManager::OnMessageReceived (TBMessage *msg)
{
   if (msg->message == messageShow)
   {
      TBWidget* tipped_widget = GetTippedWidget();
      TTMsgParam* param = static_cast<TTMsgParam*>(msg->data->v1.GetObject());
      if (tipped_widget == param->m_hovered) 
      {
         KillToolTip();

         m_tooltip = new TBTooltipWindow(tipped_widget);

         int x = TBWidget::pointer_move_widget_x;
         int y = TBWidget::pointer_move_widget_y;
         TBWidget::hovered_widget->ConvertToRoot(x, y);
         y += tooltip_point_offset_y;

         m_tooltip->Show(x,  y);

         TBMessageData* msg_data = new TBMessageData();
         msg_data->v1.SetObject(new TTMsgParam(m_tooltip));
         PostMessageDelayed(messageHide, msg_data, tooltip_show_duration_ms);
      }
   }else if (msg->message == messageHide)
   {
      TTMsgParam* param = static_cast<TTMsgParam*>(msg->data->v1.GetObject());
      if (m_tooltip == param->m_hovered)
         KillToolTip();
   }
}

