// Copyright (c) 2012 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CEF_LIBCEF_BROWSER_ALLOY_ALLOY_BROWSER_HOST_IMPL_H_
#define CEF_LIBCEF_BROWSER_ALLOY_ALLOY_BROWSER_HOST_IMPL_H_
#pragma once

#include <map>
#include <string>
#include <vector>

#include "include/cef_browser.h"
#include "include/cef_client.h"
#include "include/cef_frame.h"
#include "libcef/browser/browser_host_base.h"
#include "libcef/browser/browser_info.h"
#include "libcef/browser/file_dialog_manager.h"
#include "libcef/browser/frame_host_impl.h"
#include "libcef/browser/javascript_dialog_manager.h"
#include "libcef/browser/menu_manager.h"
#include "libcef/browser/request_context_impl.h"

#include "base/strings/string16.h"
#include "base/synchronization/lock.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_observer.h"
#include "extensions/common/view_type.h"

class CefAudioCapturer;
class CefBrowserInfo;
class CefDevToolsManager;
class SiteInstance;

// CefBrowser implementation for the alloy runtime. Method calls are delegated
// to the CefPlatformDelegate or the WebContents as appropriate. All methods are
// thread-safe unless otherwise indicated.
//
// WebContentsDelegate: Interface for handling WebContents delegations. There is
// a one-to-one relationship between AlloyBrowserHostImpl and WebContents
// instances.
//
// WebContentsObserver: Interface for observing WebContents notifications and
// IPC messages. There is a one-to-one relationship between WebContents and
// RenderViewHost instances. IPC messages received by the RenderViewHost will be
// forwarded to this WebContentsObserver implementation via WebContents. IPC
// messages sent using AlloyBrowserHostImpl::Send() will be forwarded to the
// RenderViewHost (after posting to the UI thread if necessary). Use
// WebContentsObserver::routing_id() when sending IPC messages.
class AlloyBrowserHostImpl : public CefBrowserHostBase,
                             public content::WebContentsDelegate,
                             public content::WebContentsObserver {
 public:
  // Used for handling the response to command messages.
  class CommandResponseHandler : public virtual CefBaseRefCounted {
   public:
    virtual void OnResponse(const std::string& response) = 0;
  };

  ~AlloyBrowserHostImpl() override;

  // Create a new AlloyBrowserHostImpl instance with owned WebContents.
  static CefRefPtr<AlloyBrowserHostImpl> Create(
      CefBrowserCreateParams& create_params);

  // Returns the browser associated with the specified RenderViewHost.
  static CefRefPtr<AlloyBrowserHostImpl> GetBrowserForHost(
      const content::RenderViewHost* host);
  // Returns the browser associated with the specified RenderFrameHost.
  static CefRefPtr<AlloyBrowserHostImpl> GetBrowserForHost(
      const content::RenderFrameHost* host);
  // Returns the browser associated with the specified WebContents.
  static CefRefPtr<AlloyBrowserHostImpl> GetBrowserForContents(
      const content::WebContents* contents);
  // Returns the browser associated with the specified FrameTreeNode ID.
  static CefRefPtr<AlloyBrowserHostImpl> GetBrowserForFrameTreeNode(
      int frame_tree_node_id);
  // Returns the browser associated with the specified frame routing IDs.
  static CefRefPtr<AlloyBrowserHostImpl> GetBrowserForFrameRoute(
      int render_process_id,
      int render_routing_id);

  // CefBrowserHost methods.
  void CloseBrowser(bool force_close) override;
  bool TryCloseBrowser() override;
  void SetFocus(bool focus) override;
  CefWindowHandle GetWindowHandle() override;
  CefWindowHandle GetOpenerWindowHandle() override;
  bool HasView() override;
  double GetZoomLevel() override;
  void SetZoomLevel(double zoomLevel) override;
  void RunFileDialog(FileDialogMode mode,
                     const CefString& title,
                     const CefString& default_file_path,
                     const std::vector<CefString>& accept_filters,
                     int selected_accept_filter,
                     CefRefPtr<CefRunFileDialogCallback> callback) override;
  void Print() override;
  void PrintToPDF(const CefString& path,
                  const CefPdfPrintSettings& settings,
                  CefRefPtr<CefPdfPrintCallback> callback) override;
  void Find(int identifier,
            const CefString& searchText,
            bool forward,
            bool matchCase,
            bool findNext) override;
  void StopFinding(bool clearSelection) override;
  void ShowDevTools(const CefWindowInfo& windowInfo,
                    CefRefPtr<CefClient> client,
                    const CefBrowserSettings& settings,
                    const CefPoint& inspect_element_at) override;
  void CloseDevTools() override;
  bool HasDevTools() override;
  bool SendDevToolsMessage(const void* message, size_t message_size) override;
  int ExecuteDevToolsMethod(int message_id,
                            const CefString& method,
                            CefRefPtr<CefDictionaryValue> params) override;
  CefRefPtr<CefRegistration> AddDevToolsMessageObserver(
      CefRefPtr<CefDevToolsMessageObserver> observer) override;
  bool IsWindowRenderingDisabled() override;
  void WasResized() override;
  void WasHidden(bool hidden) override;
  void NotifyScreenInfoChanged() override;
  void Invalidate(PaintElementType type) override;
  void SendExternalBeginFrame() override;
  void SendTouchEvent(const CefTouchEvent& event) override;
  void SendFocusEvent(bool setFocus) override;
  void SendCaptureLostEvent() override;
  void NotifyMoveOrResizeStarted() override;
  int GetWindowlessFrameRate() override;
  void SetWindowlessFrameRate(int frame_rate) override;
  void ImeSetComposition(const CefString& text,
                         const std::vector<CefCompositionUnderline>& underlines,
                         const CefRange& replacement_range,
                         const CefRange& selection_range) override;
  void ImeCommitText(const CefString& text,
                     const CefRange& replacement_range,
                     int relative_cursor_pos) override;
  void ImeFinishComposingText(bool keep_selection) override;
  void ImeCancelComposition() override;
  void DragTargetDragEnter(CefRefPtr<CefDragData> drag_data,
                           const CefMouseEvent& event,
                           DragOperationsMask allowed_ops) override;
  void DragTargetDragOver(const CefMouseEvent& event,
                          DragOperationsMask allowed_ops) override;
  void DragTargetDragLeave() override;
  void DragTargetDrop(const CefMouseEvent& event) override;
  void DragSourceSystemDragEnded() override;
  void DragSourceEndedAt(int x, int y, DragOperationsMask op) override;
  void SetAudioMuted(bool mute) override;
  bool IsAudioMuted() override;
  void SetAccessibilityState(cef_state_t accessibility_state) override;
  void SetAutoResizeEnabled(bool enabled,
                            const CefSize& min_size,
                            const CefSize& max_size) override;
  CefRefPtr<CefExtension> GetExtension() override;
  bool IsBackgroundHost() override;

  // Returns true if windowless rendering is enabled.
  bool IsWindowless() const;

  // Returns true if this browser is views-hosted.
  bool IsViewsHosted() const;

  // Returns true if this browser supports picture-in-picture.
  bool IsPictureInPictureSupported() const;

  // Called when the OS window hosting the browser is destroyed.
  void WindowDestroyed();

  // Destroy the browser members. This method should only be called after the
  // native browser window is not longer processing messages.
  void DestroyBrowser() override;

  // Cancel display of the context menu, if any.
  void CancelContextMenu();

#if defined(USE_AURA)
  // Returns the Widget owner for the browser window. Only used with windowed
  // rendering.
  views::Widget* GetWindowWidget() const;

  // Returns the BrowserView associated with this browser. Only used with views-
  // based browsers.
  CefRefPtr<CefBrowserView> GetBrowserView() const;
#endif

  bool MaybeAllowNavigation(content::RenderFrameHost* opener,
                            bool is_guest_view,
                            const content::OpenURLParams& params) override;

  // Convert from view coordinates to screen coordinates. Potential display
  // scaling will be applied to the result.
  gfx::Point GetScreenPoint(const gfx::Point& view) const;

  void StartDragging(const content::DropData& drop_data,
                     blink::DragOperationsMask allowed_ops,
                     const gfx::ImageSkia& image,
                     const gfx::Vector2d& image_offset,
                     const blink::mojom::DragEventSourceInfo& event_info,
                     content::RenderWidgetHostImpl* source_rwh);
  void UpdateDragCursor(blink::DragOperation operation);

  // Thread safe accessors.
  SkColor GetBackgroundColor() const;

  // Accessors that must be called on the UI thread.
  extensions::ExtensionHost* GetExtensionHost() const;

  void OnSetFocus(cef_focus_source_t source) override;

  // Run the file chooser dialog specified by |params|. Only a single dialog may
  // be pending at any given time. |callback| will be executed asynchronously
  // after the dialog is dismissed or if another dialog is already pending.
  void RunFileChooser(const CefFileDialogRunner::FileChooserParams& params,
                      CefFileDialogRunner::RunFileChooserCallback callback);

  bool HandleContextMenu(content::WebContents* web_contents,
                         const content::ContextMenuParams& params);

  enum DestructionState {
    DESTRUCTION_STATE_NONE = 0,
    DESTRUCTION_STATE_PENDING,
    DESTRUCTION_STATE_ACCEPTED,
    DESTRUCTION_STATE_COMPLETED
  };
  DestructionState destruction_state() const { return destruction_state_; }

  // content::WebContentsDelegate methods.
  content::WebContents* OpenURLFromTab(
      content::WebContents* source,
      const content::OpenURLParams& params) override;
  bool ShouldTransferNavigation(bool is_main_frame_navigation) override;
  void AddNewContents(content::WebContents* source,
                      std::unique_ptr<content::WebContents> new_contents,
                      const GURL& target_url,
                      WindowOpenDisposition disposition,
                      const gfx::Rect& initial_rect,
                      bool user_gesture,
                      bool* was_blocked) override;
  void LoadingStateChanged(content::WebContents* source,
                           bool to_different_document) override;
  void CloseContents(content::WebContents* source) override;
  void UpdateTargetURL(content::WebContents* source, const GURL& url) override;
  bool DidAddMessageToConsole(content::WebContents* source,
                              blink::mojom::ConsoleMessageLevel log_level,
                              const base::string16& message,
                              int32_t line_no,
                              const base::string16& source_id) override;
  void BeforeUnloadFired(content::WebContents* source,
                         bool proceed,
                         bool* proceed_to_fire_unload) override;
  bool TakeFocus(content::WebContents* source, bool reverse) override;
  bool HandleContextMenu(content::RenderFrameHost* render_frame_host,
                         const content::ContextMenuParams& params) override;
  content::KeyboardEventProcessingResult PreHandleKeyboardEvent(
      content::WebContents* source,
      const content::NativeWebKeyboardEvent& event) override;
  bool HandleKeyboardEvent(
      content::WebContents* source,
      const content::NativeWebKeyboardEvent& event) override;
  bool PreHandleGestureEvent(content::WebContents* source,
                             const blink::WebGestureEvent& event) override;
  bool CanDragEnter(content::WebContents* source,
                    const content::DropData& data,
                    blink::DragOperationsMask operations_allowed) override;
  void GetCustomWebContentsView(
      content::WebContents* web_contents,
      const GURL& target_url,
      int opener_render_process_id,
      int opener_render_frame_id,
      content::WebContentsView** view,
      content::RenderViewHostDelegateView** delegate_view) override;
  void WebContentsCreated(content::WebContents* source_contents,
                          int opener_render_process_id,
                          int opener_render_frame_id,
                          const std::string& frame_name,
                          const GURL& target_url,
                          content::WebContents* new_contents) override;
  void DidNavigateMainFramePostCommit(
      content::WebContents* web_contents) override;
  content::JavaScriptDialogManager* GetJavaScriptDialogManager(
      content::WebContents* source) override;
  void RunFileChooser(content::RenderFrameHost* render_frame_host,
                      scoped_refptr<content::FileSelectListener> listener,
                      const blink::mojom::FileChooserParams& params) override;
  bool EmbedsFullscreenWidget() override;
  void EnterFullscreenModeForTab(
      content::RenderFrameHost* requesting_frame,
      const blink::mojom::FullscreenOptions& options) override;
  void ExitFullscreenModeForTab(content::WebContents* web_contents) override;
  bool IsFullscreenForTabOrPending(
      const content::WebContents* web_contents) override;
  blink::mojom::DisplayMode GetDisplayMode(
      const content::WebContents* web_contents) override;
  void FindReply(content::WebContents* web_contents,
                 int request_id,
                 int number_of_matches,
                 const gfx::Rect& selection_rect,
                 int active_match_ordinal,
                 bool final_update) override;
  void UpdatePreferredSize(content::WebContents* source,
                           const gfx::Size& pref_size) override;
  void ResizeDueToAutoResize(content::WebContents* source,
                             const gfx::Size& new_size) override;
  void RequestMediaAccessPermission(
      content::WebContents* web_contents,
      const content::MediaStreamRequest& request,
      content::MediaResponseCallback callback) override;
  bool CheckMediaAccessPermission(content::RenderFrameHost* render_frame_host,
                                  const GURL& security_origin,
                                  blink::mojom::MediaStreamType type) override;
  bool IsNeverComposited(content::WebContents* web_contents) override;
  content::PictureInPictureResult EnterPictureInPicture(
      content::WebContents* web_contents,
      const viz::SurfaceId& surface_id,
      const gfx::Size& natural_size) override;
  void ExitPictureInPicture() override;

  // content::WebContentsObserver methods.
  using content::WebContentsObserver::BeforeUnloadFired;
  void RenderViewCreated(content::RenderViewHost* render_view_host) override;
  void RenderViewReady() override;
  void DidFinishNavigation(
      content::NavigationHandle* navigation_handle) override;
  void OnAudioStateChanged(bool audible) override;
  void AccessibilityEventReceived(
      const content::AXEventNotificationDetails& content_event_bundle) override;
  void AccessibilityLocationChangesReceived(
      const std::vector<content::AXLocationChangeNotificationDetails>& locData)
      override;
  void WebContentsDestroyed() override;

 private:
  friend class CefBrowserPlatformDelegateAlloy;

  static CefRefPtr<AlloyBrowserHostImpl> CreateInternal(
      const CefBrowserSettings& settings,
      CefRefPtr<CefClient> client,
      content::WebContents* web_contents,
      bool own_web_contents,
      scoped_refptr<CefBrowserInfo> browser_info,
      CefRefPtr<AlloyBrowserHostImpl> opener,
      bool is_devtools_popup,
      CefRefPtr<CefRequestContextImpl> request_context,
      std::unique_ptr<CefBrowserPlatformDelegate> platform_delegate,
      CefRefPtr<CefExtension> extension);

  AlloyBrowserHostImpl(
      const CefBrowserSettings& settings,
      CefRefPtr<CefClient> client,
      content::WebContents* web_contents,
      scoped_refptr<CefBrowserInfo> browser_info,
      CefRefPtr<AlloyBrowserHostImpl> opener,
      CefRefPtr<CefRequestContextImpl> request_context,
      std::unique_ptr<CefBrowserPlatformDelegate> platform_delegate,
      CefRefPtr<CefExtension> extension);

  // Give the platform delegate an opportunity to create the host window.
  bool CreateHostWindow();

  // Create the CefFileDialogManager if it doesn't already exist.
  void EnsureFileDialogManager();

  void StartAudioCapturer();
  void OnRecentlyAudibleTimerFired();

  bool EnsureDevToolsManager();
  void InitializeDevToolsRegistrationOnUIThread(
      CefRefPtr<CefRegistration> registration);

  void SetFocusInternal(bool focus);

  CefWindowHandle opener_;
  const bool is_windowless_;
  const bool is_views_hosted_;
  CefWindowHandle host_window_handle_ = kNullWindowHandle;
  CefRefPtr<CefExtension> extension_;
  bool is_background_host_ = false;

  // Represents the current browser destruction state. Only accessed on the UI
  // thread.
  DestructionState destruction_state_ = DESTRUCTION_STATE_NONE;

  // True if the OS window hosting the browser has been destroyed. Only accessed
  // on the UI thread.
  bool window_destroyed_ = false;

  // Used for creating and managing file dialogs.
  std::unique_ptr<CefFileDialogManager> file_dialog_manager_;

  // Used for creating and managing JavaScript dialogs.
  std::unique_ptr<CefJavaScriptDialogManager> javascript_dialog_manager_;

  // Used for creating and managing context menus.
  std::unique_ptr<CefMenuManager> menu_manager_;

  // Used for creating and managing DevTools instances.
  std::unique_ptr<CefDevToolsManager> devtools_manager_;

  // Used for capturing audio for CefAudioHandler.
  std::unique_ptr<CefAudioCapturer> audio_capturer_;

  // Timer for determining when "recently audible" transitions to false. This
  // starts running when a tab stops being audible, and is canceled if it starts
  // being audible again before it fires.
  base::OneShotTimer recently_audible_timer_;
};

#endif  // CEF_LIBCEF_BROWSER_ALLOY_ALLOY_BROWSER_HOST_IMPL_H_
