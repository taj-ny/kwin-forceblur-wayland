/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2015 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#ifndef KWIN_ABSTRACT_CLIENT_H
#define KWIN_ABSTRACT_CLIENT_H

#include "toplevel.h"
#include "options.h"
#include "rules.h"
#include "tabgroup.h"

#include <memory>

namespace KWayland
{
namespace Server
{
class PlasmaWindowInterface;
}
}

namespace KDecoration2
{
class Decoration;
}

namespace KWin
{

namespace TabBox
{
class TabBoxClientImpl;
}

namespace Decoration
{
class DecorationPalette;
}

class KWIN_EXPORT AbstractClient : public Toplevel
{
    Q_OBJECT
    /**
     * Whether this Client is fullScreen. A Client might either be fullScreen due to the _NET_WM property
     * or through a legacy support hack. The fullScreen state can only be changed if the Client does not
     * use the legacy hack. To be sure whether the state changed, connect to the notify signal.
     **/
    Q_PROPERTY(bool fullScreen READ isFullScreen WRITE setFullScreen NOTIFY fullScreenChanged)
    /**
     * Whether the Client can be set to fullScreen. The property is evaluated each time it is invoked.
     * Because of that there is no notify signal.
     **/
    Q_PROPERTY(bool fullScreenable READ isFullScreenable)
    /**
     * Whether this Client is the currently visible Client in its Client Group (Window Tabs).
     * For change connect to the visibleChanged signal on the Client's Group.
     **/
    Q_PROPERTY(bool isCurrentTab READ isCurrentTab)
    /**
     * Whether this Client is active or not. Use Workspace::activateClient() to activate a Client.
     * @see Workspace::activateClient
     **/
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
    /**
     * The desktop this Client is on. If the Client is on all desktops the property has value -1.
     **/
    Q_PROPERTY(int desktop READ desktop WRITE setDesktop NOTIFY desktopChanged)
    /**
     * Whether the Client is on all desktops. That is desktop is -1.
     **/
    Q_PROPERTY(bool onAllDesktops READ isOnAllDesktops WRITE setOnAllDesktops NOTIFY desktopChanged)
    /**
     * Indicates that the window should not be included on a taskbar.
     **/
    Q_PROPERTY(bool skipTaskbar READ skipTaskbar WRITE setSkipTaskbar NOTIFY skipTaskbarChanged)
    /**
     * Indicates that the window should not be included on a Pager.
     **/
    Q_PROPERTY(bool skipPager READ skipPager WRITE setSkipPager NOTIFY skipPagerChanged)
    /**
     * Whether the Client should be excluded from window switching effects.
     **/
    Q_PROPERTY(bool skipSwitcher READ skipSwitcher WRITE setSkipSwitcher NOTIFY skipSwitcherChanged)
    /**
     * Whether the window can be closed by the user. The value is evaluated each time the getter is called.
     * Because of that no changed signal is provided.
     **/
    Q_PROPERTY(bool closeable READ isCloseable)
    Q_PROPERTY(QIcon icon READ icon NOTIFY iconChanged)
    /**
     * Whether the Client is set to be kept above other windows.
     **/
    Q_PROPERTY(bool keepAbove READ keepAbove WRITE setKeepAbove NOTIFY keepAboveChanged)
    /**
     * Whether the Client is set to be kept below other windows.
     **/
    Q_PROPERTY(bool keepBelow READ keepBelow WRITE setKeepBelow NOTIFY keepBelowChanged)
    /**
     * Whether the Client can be shaded. The property is evaluated each time it is invoked.
     * Because of that there is no notify signal.
     **/
    Q_PROPERTY(bool shadeable READ isShadeable)
    /**
     * Whether the Client is shaded.
     **/
    Q_PROPERTY(bool shade READ isShade WRITE setShade NOTIFY shadeChanged)
    /**
     * Whether the Client can be minimized. The property is evaluated each time it is invoked.
     * Because of that there is no notify signal.
     **/
    Q_PROPERTY(bool minimizable READ isMinimizable)
    /**
     * Whether the Client is minimized.
     **/
    Q_PROPERTY(bool minimized READ isMinimized WRITE setMinimized NOTIFY minimizedChanged)
    /**
     * The optional geometry representing the minimized Client in e.g a taskbar.
     * See _NET_WM_ICON_GEOMETRY at http://standards.freedesktop.org/wm-spec/wm-spec-latest.html .
     * The value is evaluated each time the getter is called.
     * Because of that no changed signal is provided.
     **/
    Q_PROPERTY(QRect iconGeometry READ iconGeometry)
    /**
     * Returns whether the window is any of special windows types (desktop, dock, splash, ...),
     * i.e. window types that usually don't have a window frame and the user does not use window
     * management (moving, raising,...) on them.
     * The value is evaluated each time the getter is called.
     * Because of that no changed signal is provided.
     **/
    Q_PROPERTY(bool specialWindow READ isSpecialWindow)
    /**
     * Whether window state _NET_WM_STATE_DEMANDS_ATTENTION is set. This state indicates that some
     * action in or with the window happened. For example, it may be set by the Window Manager if
     * the window requested activation but the Window Manager refused it, or the application may set
     * it if it finished some work. This state may be set by both the Client and the Window Manager.
     * It should be unset by the Window Manager when it decides the window got the required attention
     * (usually, that it got activated).
     **/
    Q_PROPERTY(bool demandsAttention READ isDemandingAttention WRITE demandAttention NOTIFY demandsAttentionChanged)
    /**
     * The Caption of the Client. Read from WM_NAME property together with a suffix for hostname and shortcut.
     * To read only the caption as provided by WM_NAME, use the getter with an additional @c false value.
     **/
    Q_PROPERTY(QString caption READ caption NOTIFY captionChanged)
    /**
     * Minimum size as specified in WM_NORMAL_HINTS
     **/
    Q_PROPERTY(QSize minSize READ minSize)
    /**
     * Maximum size as specified in WM_NORMAL_HINTS
     **/
    Q_PROPERTY(QSize maxSize READ maxSize)
    /**
     * Whether the Client can accept keyboard focus.
     * The value is evaluated each time the getter is called.
     * Because of that no changed signal is provided.
     **/
    Q_PROPERTY(bool wantsInput READ wantsInput)
    /**
     * Whether the Client is a transient Window to another Window.
     * @see transientFor
     **/
    Q_PROPERTY(bool transient READ isTransient NOTIFY transientChanged)
    /**
     * The Client to which this Client is a transient if any.
     **/
    Q_PROPERTY(KWin::AbstractClient *transientFor READ transientFor NOTIFY transientChanged)
    /**
     * Whether the Client represents a modal window.
     **/
    Q_PROPERTY(bool modal READ isModal NOTIFY modalChanged)
    /**
     * The geometry of this Client. Be aware that depending on resize mode the geometryChanged signal
     * might be emitted at each resize step or only at the end of the resize operation.
     **/
    Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry)
    /**
     * Whether the Client is currently being moved by the user.
     * Notify signal is emitted when the Client starts or ends move/resize mode.
     **/
    Q_PROPERTY(bool move READ isMove NOTIFY moveResizedChanged)
    /**
     * Whether the Client is currently being resized by the user.
     * Notify signal is emitted when the Client starts or ends move/resize mode.
     **/
    Q_PROPERTY(bool resize READ isResize NOTIFY moveResizedChanged)
public:
    virtual ~AbstractClient();

    QWeakPointer<TabBox::TabBoxClientImpl> tabBoxClient() const {
        return m_tabBoxClient.toWeakRef();
    }
    bool isFirstInTabBox() const {
        return m_firstInTabBox;
    }
    bool skipSwitcher() const {
        return m_skipSwitcher;
    }
    void setSkipSwitcher(bool set);

    bool skipTaskbar() const {
        return m_skipTaskbar;
    }
    void setSkipTaskbar(bool set);
    void setOriginalSkipTaskbar(bool set);
    bool originalSkipTaskbar() const {
        return m_originalSkipTaskbar;
    }

    bool skipPager() const {
        return m_skipPager;
    }
    void setSkipPager(bool set);

    const QIcon &icon() const {
        return m_icon;
    }

    bool isActive() const {
        return m_active;
    }
    /**
     * Sets the client's active state to \a act.
     *
     * This function does only change the visual appearance of the client,
     * it does not change the focus setting. Use
     * Workspace::activateClient() or Workspace::requestFocus() instead.
     *
     * If a client receives or looses the focus, it calls setActive() on
     * its own.
     **/
    void setActive(bool);

    bool keepAbove() const {
        return m_keepAbove;
    }
    void setKeepAbove(bool);
    bool keepBelow() const {
        return m_keepBelow;
    }
    void setKeepBelow(bool);

    void demandAttention(bool set = true);
    bool isDemandingAttention() const {
        return m_demandsAttention;
    }

    void cancelAutoRaise();

    bool wantsTabFocus() const;

    virtual void updateMouseGrab();
    virtual QString caption(bool full = true, bool stripped = false) const = 0;
    virtual bool isCloseable() const = 0;
    // TODO: remove boolean trap
    virtual bool isShown(bool shaded_is_shown) const = 0;
    // TODO: remove boolean trap
    virtual void hideClient(bool hide) = 0;
    virtual bool isFullScreenable() const = 0;
    virtual bool isFullScreen() const = 0;
    // TODO: remove boolean trap
    virtual AbstractClient *findModal(bool allow_itself = false) = 0;
    virtual bool isTransient() const;
    /**
     * @returns Whether there is a hint available to place the AbstractClient on it's parent, default @c false.
     * @see transientPlacementHint
     **/
    virtual bool hasTransientPlacementHint() const;
    /**
     * @returns The recommended position of the transient in parent coordinates
     **/
    virtual QPoint transientPlacementHint() const;
    const AbstractClient* transientFor() const;
    AbstractClient* transientFor();
    /**
     * @returns @c true if c is the transient_for window for this client,
     *  or recursively the transient_for window
     * @todo: remove boolean trap
     **/
    virtual bool hasTransient(const AbstractClient* c, bool indirect) const;
    const QList<AbstractClient*>& transients() const; // Is not indirect
    virtual void removeTransient(AbstractClient* cl);
    virtual QList<AbstractClient*> mainClients() const; // Call once before loop , is not indirect
    QList<AbstractClient*> allMainClients() const; // Call once before loop , is indirect
    /**
     * Returns true for "special" windows and false for windows which are "normal"
     * (normal=window which has a border, can be moved by the user, can be closed, etc.)
     * true for Desktop, Dock, Splash, Override and TopMenu (and Toolbar??? - for now)
     * false for Normal, Dialog, Utility and Menu (and Toolbar??? - not yet) TODO
     */
    bool isSpecialWindow() const;
    void sendToScreen(int screen);
    virtual const QKeySequence &shortcut() const  = 0;
    virtual void setShortcut(const QString &cut) = 0;
    virtual bool performMouseCommand(Options::MouseCommand, const QPoint &globalPos);
    void setOnAllDesktops(bool set);
    void setDesktop(int);
    int desktop() const override {
        return m_desktop;
    }
    void setMinimized(bool set);
    /**
    * Minimizes this client plus its transients
    */
    void minimize(bool avoid_animation = false);
    void unminimize(bool avoid_animation = false);
    bool isMinimized() const {
        return m_minimized;
    }
    virtual void setFullScreen(bool set, bool user = true) = 0;
    virtual TabGroup *tabGroup() const;
    Q_INVOKABLE virtual bool untab(const QRect &toGeometry = QRect(), bool clientRemoved = false);
    virtual bool isCurrentTab() const;
    virtual QRect geometryRestore() const = 0;
    virtual MaximizeMode maximizeMode() const = 0;
    void maximize(MaximizeMode);
    void setMaximize(bool vertically, bool horizontally);
    virtual bool noBorder() const = 0;
    virtual void setNoBorder(bool set) = 0;
    virtual void blockActivityUpdates(bool b = true) = 0;
    QPalette palette() const;
    const Decoration::DecorationPalette *decorationPalette() const;
    virtual bool isResizable() const = 0;
    virtual bool isMovable() const = 0;
    virtual bool isMovableAcrossScreens() const = 0;
    /**
     * @c true only for @c ShadeNormal
     **/
    bool isShade() const {
        return shadeMode() == ShadeNormal;
    }
    /**
     * Default implementation returns @c ShadeNone
     **/
    virtual ShadeMode shadeMode() const; // Prefer isShade()
    void setShade(bool set);
    /**
     * Default implementation does nothing
     **/
    virtual void setShade(ShadeMode mode);
    /**
     * Whether the Client can be shaded. Default implementation returns @c false.
     **/
    virtual bool isShadeable() const;
    virtual bool isMaximizable() const = 0;
    virtual bool isMinimizable() const = 0;
    virtual QRect iconGeometry() const = 0;
    virtual bool userCanSetFullScreen() const = 0;
    virtual bool userCanSetNoBorder() const = 0;
    virtual void setOnAllActivities(bool set) = 0;
    virtual const WindowRules* rules() const = 0;
    virtual void takeFocus() = 0;
    virtual bool wantsInput() const = 0;
    void checkWorkspacePosition(QRect oldGeometry = QRect(), int oldDesktop = -2,  QRect oldClientGeometry = QRect());
    virtual xcb_timestamp_t userTime() const;
    virtual void updateWindowRules(Rules::Types selection) = 0;

    void growHorizontal();
    void shrinkHorizontal();
    void growVertical();
    void shrinkVertical();
    void updateMoveResize(const QPointF &currentGlobalCursor);
    /**
     * Ends move resize when all pointer buttons are up again.
     **/
    void endMoveResize();
    void keyPressEvent(uint key_code);

    /**
     * These values represent positions inside an area
     */
    enum Position {
        // without prefix, they'd conflict with Qt::TopLeftCorner etc. :(
        PositionCenter         = 0x00,
        PositionLeft           = 0x01,
        PositionRight          = 0x02,
        PositionTop            = 0x04,
        PositionBottom         = 0x08,
        PositionTopLeft        = PositionLeft | PositionTop,
        PositionTopRight       = PositionRight | PositionTop,
        PositionBottomLeft     = PositionLeft | PositionBottom,
        PositionBottomRight    = PositionRight | PositionBottom
    };
    Position titlebarPosition() const;

    // a helper for the workspace window packing. tests for screen validity and updates since in maximization case as with normal moving
    void packTo(int left, int top);

    enum QuickTileFlag {
        QuickTileNone = 0,
        QuickTileLeft = 1,
        QuickTileRight = 1<<1,
        QuickTileTop = 1<<2,
        QuickTileBottom = 1<<3,
        QuickTileHorizontal = QuickTileLeft|QuickTileRight,
        QuickTileVertical = QuickTileTop|QuickTileBottom,
        QuickTileMaximize = QuickTileLeft|QuickTileRight|QuickTileTop|QuickTileBottom
    };
    Q_DECLARE_FLAGS(QuickTileMode, QuickTileFlag)
    /** Set the quick tile mode ("snap") of this window.
     * This will also handle preserving and restoring of window geometry as necessary.
     * @param mode The tile mode (left/right) to give this window.
     */
    void setQuickTileMode(QuickTileMode mode, bool keyboard = false);
    QuickTileMode quickTileMode() const {
        return QuickTileMode(m_quickTileMode);
    }
    Layer layer() const override;
    void updateLayer();

    enum ForceGeometry_t { NormalGeometrySet, ForceGeometrySet };
    void move(int x, int y, ForceGeometry_t force = NormalGeometrySet);
    void move(const QPoint &p, ForceGeometry_t force = NormalGeometrySet);
    virtual void resizeWithChecks(int w, int h, ForceGeometry_t force = NormalGeometrySet) = 0;
    void resizeWithChecks(const QSize& s, ForceGeometry_t force = NormalGeometrySet);
    void keepInArea(QRect area, bool partial = false);
    virtual QSize minSize() const;
    virtual QSize maxSize() const;
    virtual void setGeometry(int x, int y, int w, int h, ForceGeometry_t force = NormalGeometrySet) = 0;
    void setGeometry(const QRect& r, ForceGeometry_t force = NormalGeometrySet);
    /// How to resize the window in order to obey constains (mainly aspect ratios)
    enum Sizemode {
        SizemodeAny,
        SizemodeFixedW, ///< Try not to affect width
        SizemodeFixedH, ///< Try not to affect height
        SizemodeMax ///< Try not to make it larger in either direction
    };
    /**
     *Calculate the appropriate frame size for the given client size @p wsize.
     *
     * @p wsize is adapted according to the window's size hints (minimum, maximum and incremental size changes).
     *
     * Default implementation returns the passed in @p wsize.
     */
    virtual QSize sizeForClientSize(const QSize &wsize, Sizemode mode = SizemodeAny, bool noframe = false) const;

    QSize adjustedSize(const QSize&, Sizemode mode = SizemodeAny) const;
    QSize adjustedSize() const;

    bool isMove() const {
        return isMoveResize() && moveResizePointerMode() == PositionCenter;
    }
    bool isResize() const {
        return isMoveResize() && moveResizePointerMode() != PositionCenter;
    }
    /**
     * Cursor shape for move/resize mode.
     **/
    Qt::CursorShape cursor() const {
        return m_moveResize.cursor;
    }

    virtual bool hasStrut() const;

    void setModal(bool modal);
    bool isModal() const;

    /**
     * Determines the mouse command for the given @p button in the current state.
     *
     * The @p handled argument specifies whether the button was handled or not.
     * This value should be used to determine whether the mouse button should be
     * passed to the AbstractClient or being filtered out.
     **/
    Options::MouseCommand getMouseCommand(Qt::MouseButton button, bool *handled) const;
    Options::MouseCommand getWheelCommand(Qt::Orientation orientation, bool *handled) const;

    // decoration related
    KDecoration2::Decoration *decoration() {
        return m_decoration;
    }
    const KDecoration2::Decoration *decoration() const {
        return m_decoration;
    }
    bool isDecorated() const {
        return m_decoration != nullptr;
    }

    // TODO: remove boolean trap
    static bool belongToSameApplication(const AbstractClient* c1, const AbstractClient* c2, bool active_hack = false);

public Q_SLOTS:
    virtual void closeWindow() = 0;

Q_SIGNALS:
    void fullScreenChanged();
    void skipTaskbarChanged();
    void skipPagerChanged();
    void skipSwitcherChanged();
    void iconChanged();
    void activeChanged();
    void keepAboveChanged(bool);
    void keepBelowChanged(bool);
    /**
     * Emitted whenever the demands attention state changes.
     **/
    void demandsAttentionChanged();
    void desktopPresenceChanged(KWin::AbstractClient*, int); // to be forwarded by Workspace
    void desktopChanged();
    void shadeChanged();
    void minimizedChanged();
    void clientMinimized(KWin::AbstractClient* client, bool animate);
    void clientUnminimized(KWin::AbstractClient* client, bool animate);
    void paletteChanged(const QPalette &p);
    void captionChanged();
    void clientMaximizedStateChanged(KWin::AbstractClient*, MaximizeMode);
    void clientMaximizedStateChanged(KWin::AbstractClient* c, bool h, bool v);
    void transientChanged();
    void modalChanged();
    void quickTileModeChanged();
    void moveResizedChanged();
    void moveResizeCursorChanged(Qt::CursorShape);
    void clientStartUserMovedResized(KWin::AbstractClient*);
    void clientStepUserMovedResized(KWin::AbstractClient *, const QRect&);
    void clientFinishUserMovedResized(KWin::AbstractClient*);

protected:
    AbstractClient();
    void setFirstInTabBox(bool enable) {
        m_firstInTabBox = enable;
    }
    void setIcon(const QIcon &icon);
    void startAutoRaise();
    void autoRaise();
    /**
     * Called from ::setActive once the active value got updated, but before the changed signal
     * is emitted.
     *
     * Default implementation does nothing.
     **/
    virtual void doSetActive();
    /**
     * Called from ::setKeepAbove once the keepBelow value got updated, but before the changed signal
     * is emitted.
     *
     * Default implementation does nothing.
     **/
    virtual void doSetKeepAbove();
    /**
     * Called from ::setKeepBelow once the keepBelow value got updated, but before the changed signal
     * is emitted.
     *
     * Default implementation does nothing.
     **/
    virtual void doSetKeepBelow();
    /**
     * Called from ::setDeskop once the desktop value got updated, but before the changed signal
     * is emitted.
     *
     * Default implementation does nothing.
     * @param desktop The new desktop the Client is on
     * @param was_desk The desktop the Client was on before
     **/
    virtual void doSetDesktop(int desktop, int was_desk);
    /**
     * Called from ::minimize and ::unminimize once the minimized value got updated, but before the
     * changed signal is emitted.
     *
     * Default implementation does nothig.
     **/
    virtual void doMinimize();
    // TODO: remove boolean trap
    virtual bool belongsToSameApplication(const AbstractClient *other, bool active_hack) const = 0;

    virtual void doSetSkipTaskbar();
    virtual void doSetSkipPager();

    void setupWindowManagementInterface();
    void destroyWindowManagementInterface();

    void updateColorScheme(QString path);

    void setTransientFor(AbstractClient *transientFor);
    virtual void addTransient(AbstractClient* cl);
    /**
     * Just removes the @p cl from the transients without any further checks.
     **/
    void removeTransientFromList(AbstractClient* cl);

    Layer belongsToLayer() const;
    virtual bool belongsToDesktop() const;
    void invalidateLayer();
    virtual bool isActiveFullScreen() const;
    virtual Layer layerForDock() const;

    // electric border / quick tiling
    void setElectricBorderMode(QuickTileMode mode);
    QuickTileMode electricBorderMode() const {
        return m_electricMode;
    }
    void setElectricBorderMaximizing(bool maximizing);
    bool isElectricBorderMaximizing() const {
        return m_electricMaximizing;
    }
    QRect electricBorderMaximizeGeometry(QPoint pos, int desktop);
    void updateQuickTileMode(QuickTileMode newMode) {
        m_quickTileMode = newMode;
    }

    KWayland::Server::PlasmaWindowInterface *windowManagementInterface() const {
        return m_windowManagementInterface;
    }

    // geometry handling
    void checkOffscreenPosition(QRect *geom, const QRect &screenArea);
    virtual int borderLeft() const;
    virtual int borderRight() const;
    virtual int borderTop() const;
    virtual int borderBottom() const;
    virtual void changeMaximize(bool horizontal, bool vertical, bool adjust) = 0;
    virtual void setGeometryRestore(const QRect &geo) = 0;
    /**
     * Called from move after updating the geometry. Can be reimplemented to perform specific tasks.
     * The base implementation does nothing.
     **/
    virtual void doMove(int x, int y);
    void blockGeometryUpdates(bool block);
    void blockGeometryUpdates();
    void unblockGeometryUpdates();
    bool areGeometryUpdatesBlocked() const;
    enum PendingGeometry_t {
        PendingGeometryNone,
        PendingGeometryNormal,
        PendingGeometryForced
    };
    PendingGeometry_t pendingGeometryUpdate() const;
    void setPendingGeometryUpdate(PendingGeometry_t update);
    QRect geometryBeforeUpdateBlocking() const {
        return m_geometryBeforeUpdateBlocking;
    }
    void updateGeometryBeforeUpdateBlocking();
    /**
     * Schedules a repaint for the visibleRect before and after a
     * geometry update. The current visibleRect is stored for the
     * next time this method is called as the before geometry.
     **/
    void addRepaintDuringGeometryUpdates();

    /**
     * Convenient method to update the TabGroup states if there is one present.
     * Marked as virtual as TabGroup does not yet handle AbstractClient, but only
     * subclasses of AbstractClient. Given that the default implementation does nothing.
     **/
    virtual void updateTabGroupStates(TabGroup::States states);

    /**
     * @returns whether the Client is currently in move resize mode
     **/
    bool isMoveResize() const {
        return m_moveResize.enabled;
    }
    /**
     * Sets whether the Client is in move resize mode to @p enabled.
     **/
    void setMoveResize(bool enabled) {
        m_moveResize.enabled = enabled;
    }
    /**
     * @returns whether the move resize mode is unrestricted.
     **/
    bool isUnrestrictedMoveResize() const {
        return m_moveResize.unrestricted;
    }
    /**
     * Sets whether move resize mode is unrestricted to @p set.
     **/
    void setUnrestrictedMoveResize(bool set) {
        m_moveResize.unrestricted = set;
    }
    QPoint moveOffset() const {
        return m_moveResize.offset;
    }
    void setMoveOffset(const QPoint &offset) {
        m_moveResize.offset = offset;
    }
    QPoint invertedMoveOffset() const {
        return m_moveResize.invertedOffset;
    }
    void setInvertedMoveOffset(const QPoint &offset) {
        m_moveResize.invertedOffset = offset;
    }
    QRect initialMoveResizeGeometry() const {
        return m_moveResize.initialGeometry;
    }
    /**
     * Sets the initial move resize geometry to the current geometry.
     **/
    void updateInitialMoveResizeGeometry();
    QRect moveResizeGeometry() const {
        return m_moveResize.geometry;
    }
    void setMoveResizeGeometry(const QRect &geo) {
        m_moveResize.geometry = geo;
    }
    Position moveResizePointerMode() const {
        return m_moveResize.pointer;
    }
    void setMoveResizePointerMode(Position mode) {
        m_moveResize.pointer = mode;
    }
    bool isMoveResizePointerButtonDown() const {
        return m_moveResize.buttonDown;
    }
    void setMoveResizePointerButtonDown(bool down) {
        m_moveResize.buttonDown = down;
    }
    int moveResizeStartScreen() const {
        return m_moveResize.startScreen;
    }
    void checkUnrestrictedMoveResize();
    /**
    * Sets an appropriate cursor shape for the logical mouse position.
    */
    void updateCursor();
    void startDelayedMoveResize();
    void stopDelayedMoveResize();
    bool startMoveResize();
    /**
     * Called from @link startMoveResize.
     *
     * Implementing classes should return @c false if starting move resize should
     * get aborted. In that case @link startMoveResize will also return @c false.
     *
     * Base implementation returns @c true.
     **/
    virtual bool doStartMoveResize();
    void finishMoveResize(bool cancel);
    /**
     * Leaves the move resize mode.
     *
     * Inheriting classes must invoke the base implementation which
     * ensures that the internal mode is properly ended.
     **/
    virtual void leaveMoveResize();
    virtual void positionGeometryTip();
    void performMoveResize();
    /**
     * Called from performMoveResize() after actually performing the change of geometry.
     * Implementing subclasses can perform windowing system specific handling here.
     *
     * Default implementation does nothing.
     **/
    virtual void doPerformMoveResize();
    /*
     * Checks if the mouse cursor is near the edge of the screen and if so
     * activates quick tiling or maximization
     */
    void checkQuickTilingMaximizationZones(int xroot, int yroot);
    /**
     * Whether a sync request is still pending.
     * Default implementation returns @c false.
     **/
    virtual bool isWaitingForMoveResizeSync() const;
    /**
     * Called during handling a resize. Implementing subclasses can use this
     * method to perform windowing system specific syncing.
     *
     * Default implementation does nothing.
     **/
    virtual void doResizeSync();
    void handleMoveResize(int x, int y, int x_root, int y_root);
    void handleMoveResize(const QPoint &local, const QPoint &global);

    virtual QSize resizeIncrements() const;

    /**
     * Default implementation returns PositionCenter
     **/
    virtual Position mousePosition() const;

    static bool haveResizeEffect() {
        return s_haveResizeEffect;
    }
    static void updateHaveResizeEffect();
    static void resetHaveResizeEffect() {
        s_haveResizeEffect = false;
    }

    void setDecoration(KDecoration2::Decoration *decoration) {
        m_decoration = decoration;
    }
    virtual void destroyDecoration();

private:
    void handlePaletteChange();
    QSharedPointer<TabBox::TabBoxClientImpl> m_tabBoxClient;
    bool m_firstInTabBox = false;
    bool m_skipTaskbar = false;
    /**
     * Unaffected by KWin
     **/
    bool m_originalSkipTaskbar = false;
    bool m_skipPager = false;
    bool m_skipSwitcher = false;
    QIcon m_icon;
    bool m_active = false;
    bool m_keepAbove = false;
    bool m_keepBelow = false;
    bool m_demandsAttention = false;
    bool m_minimized = false;
    QTimer *m_autoRaiseTimer = nullptr;
    int m_desktop = 0; // 0 means not on any desktop yet

    QString m_colorScheme;
    std::shared_ptr<Decoration::DecorationPalette> m_palette;
    static QHash<QString, std::weak_ptr<Decoration::DecorationPalette>> s_palettes;
    static std::shared_ptr<Decoration::DecorationPalette> s_defaultPalette;

    KWayland::Server::PlasmaWindowInterface *m_windowManagementInterface = nullptr;

    AbstractClient *m_transientFor = nullptr;
    QList<AbstractClient*> m_transients;
    bool m_modal = false;
    Layer m_layer = UnknownLayer;

    // electric border/quick tiling
    QuickTileMode m_electricMode = QuickTileNone;
    bool m_electricMaximizing = false;
    /** The quick tile mode of this window.
     */
    int m_quickTileMode = QuickTileNone;
    QTimer *m_electricMaximizingDelay = nullptr;

    // geometry
    int m_blockGeometryUpdates = 0; // > 0 = New geometry is remembered, but not actually set
    PendingGeometry_t m_pendingGeometryUpdate = PendingGeometryNone;
    friend class GeometryUpdatesBlocker;
    QRect m_visibleRectBeforeGeometryUpdate;
    QRect m_geometryBeforeUpdateBlocking;

    struct {
        bool enabled = false;
        bool unrestricted = false;
        QPoint offset;
        QPoint invertedOffset;
        QRect initialGeometry;
        QRect geometry;
        Position pointer = PositionCenter;
        bool buttonDown = false;
        Qt::CursorShape cursor = Qt::ArrowCursor;
        int startScreen = 0;
        QTimer *delayedTimer = nullptr;
    } m_moveResize;

    KDecoration2::Decoration *m_decoration = nullptr;


    static bool s_haveResizeEffect;
};

/**
 * Helper for AbstractClient::blockGeometryUpdates() being called in pairs (true/false)
 */
class GeometryUpdatesBlocker
{
public:
    explicit GeometryUpdatesBlocker(AbstractClient* c)
        : cl(c) {
        cl->blockGeometryUpdates(true);
    }
    ~GeometryUpdatesBlocker() {
        cl->blockGeometryUpdates(false);
    }

private:
    AbstractClient* cl;
};

inline void AbstractClient::move(const QPoint& p, ForceGeometry_t force)
{
    move(p.x(), p.y(), force);
}

inline void AbstractClient::resizeWithChecks(const QSize& s, AbstractClient::ForceGeometry_t force)
{
    resizeWithChecks(s.width(), s.height(), force);
}

inline void AbstractClient::setGeometry(const QRect& r, ForceGeometry_t force)
{
    setGeometry(r.x(), r.y(), r.width(), r.height(), force);
}

inline const QList<AbstractClient*>& AbstractClient::transients() const
{
    return m_transients;
}

inline bool AbstractClient::areGeometryUpdatesBlocked() const
{
    return m_blockGeometryUpdates != 0;
}

inline void AbstractClient::blockGeometryUpdates()
{
    m_blockGeometryUpdates++;
}

inline void AbstractClient::unblockGeometryUpdates()
{
    m_blockGeometryUpdates--;
}

inline AbstractClient::PendingGeometry_t AbstractClient::pendingGeometryUpdate() const
{
    return m_pendingGeometryUpdate;
}

inline void AbstractClient::setPendingGeometryUpdate(PendingGeometry_t update)
{
    m_pendingGeometryUpdate = update;
}

}

Q_DECLARE_METATYPE(KWin::AbstractClient*)
Q_DECLARE_METATYPE(QList<KWin::AbstractClient*>)
Q_DECLARE_OPERATORS_FOR_FLAGS(KWin::AbstractClient::QuickTileMode)

#endif
