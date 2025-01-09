#pragma once 
#include <juce_gui_extra/juce_gui_extra.h>
#include "../choc/gui/choc_WebView.h"

struct WebViewInComponent
{

    WebViewInComponent(); 
    ~WebViewInComponent(); 
    
    choc::ui::WebView webView;
    std::unique_ptr<juce::Component> webViewHolder; 
};