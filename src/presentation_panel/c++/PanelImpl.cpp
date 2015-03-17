/*
 * Copyright (C) 2015 Moonlight Desktop Environment Team
 * Authors:
 *      Alexis López Zubieta
 * This file is part of Moonlight Desktop Environment.
 *
 * Moonlight Desktop Environment is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Moonlight Desktop Environment is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Moonlight Desktop Environment. If not, see <http://www.gnu.org/licenses/>.
 */

#include "module_config.h"
#include "PanelImpl.h"
#include "core/ModuleSettings.h"

#include <qt5xdg/XdgMenu>

#include <usModule.h>
#include <usGetModuleContext.h>
#include <usServiceReference.h>
#include <usServiceException.h>

#include <KWindowSystem>

#include <QDebug>
#include <QMenu>
#include <QWidget>
#include <QMargins>
#include <QWindow>
#include <QPointer>
#include <QHBoxLayout>
#include <QDesktopWidget>
#include <QApplication>

using namespace us;

PanelImpl::PanelImpl(QWidget *parent) :
QWidget(parent), m_Desktop(-1), m_WidgetTracker(this) {

    setWindowFlags(Qt::WindowDoesNotAcceptFocus);
    setAttribute(Qt::WA_AlwaysShowToolTips);
    adjustSizeToScreen();
    setupWindowFlags();
    us::ModuleContext * context = us::GetModuleContext();
    
    module_settings = ModuleSettings::getModuleSettings(context);
    //Write confs into configuration file
    module_settings->setValue("buttonMarginLeft", 10);
    module_settings->setValue("buttonMarginTop", 6);
    module_settings->setValue("buttonMarginRight", 10);
    module_settings->setValue("buttonMarginBottom", 6);
    
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(adjustSizeToScreen()));
}

PanelImpl::~PanelImpl() {
    delete module_settings;
}

bool PanelImpl::event(QEvent *event) {
    switch (event->type()) {
        case QEvent::Show:
            requestExclusiveScreenArea();
            break;
        case QEvent::ContextMenu:
            break;
                    
        case QEvent::LayoutRequest:
//            qDebug() << MODULE_NAME_STR << ": desktop layout changed.";
            adjustSizeToScreen();
            break;

        case QEvent::WinIdChange:
//            qDebug() << MODULE_NAME_STR << ": Wid changed.";
            setupWindowFlags();
            adjustSizeToScreen();
        default:
            break;
    }
    return QWidget::event(event);
}

void PanelImpl::adjustSizeToScreen() {
    // TODO: Hard coded values
    const QRect screenGeometry = QApplication::desktop()->screenGeometry(this);
    m_Geometry.setWidth(screenGeometry.width());
    m_Geometry.setHeight(36);
    m_Geometry.moveBottomLeft(screenGeometry.bottomLeft());

    QWidget::setGeometry(m_Geometry);

//    qDebug() << MODULE_NAME_STR << ": Adjusting size to: " << m_Geometry.size();
//    qDebug() << MODULE_NAME_STR << ": Adjusting position to: " << m_Geometry.topLeft();
//    qDebug() << MODULE_NAME_STR << ": resulting geometry: " << geometry();

    // Update reserved screen area on resize
    requestExclusiveScreenArea();
}

inline void PanelImpl::setupWindowFlags() {
    if (m_Desktop < 0)
        KWindowSystem::setOnAllDesktops(effectiveWinId(), true);
    else {
        KWindowSystem::setOnAllDesktops(effectiveWinId(), false);
        KWindowSystem::setOnDesktop(effectiveWinId(), m_Desktop);
    }

    KWindowSystem::setType(effectiveWinId(), NET::Dock);
}

void PanelImpl::requestExclusiveScreenArea() {
    // TODO: Look for an alternate implementation inside the KF5.
    WId wid = effectiveWinId();
    if (wid == 0 || !isVisible())
        return;

    const QRect wholeScreen = QApplication::desktop()->geometry();
    KWindowSystem::setStrut(wid, 0, 0, 0,  wholeScreen.bottom() - m_Geometry.y());
}

void PanelImpl::addWidgetFactory(presentation_panel::WidgetFactory* widgetFactory) {
    if (widgetFactory == NULL) {
        qWarning() << MODULE_NAME_STR << " : adding a NULL object widget factory.";
        return;
    }

    QString name = widgetFactory->name();
    if (m_Widgets.contains(name)) {
        qDebug() << MODULE_NAME_STR << " : " << name << " already registered.";
        return;
    }

    QWidget * widget = widgetFactory->build(MODULE_NAME_STR, this);
    if (widget == NULL) {
        qDebug() << MODULE_NAME_STR << " : " << name << " widget wasn't built.";
        return;
    }

    m_Widgets.insert(name, widget);
    m_Factories.insert(name, widgetFactory);
    
    updateLayout();
}

void PanelImpl::removeWidgetFactory(presentation_panel::WidgetFactory* widgetFactory) {
    if (widgetFactory == NULL) {
        qWarning() << MODULE_NAME_STR << ": traing to remove a NULL widget factory.";
        return;
    }
    QString name = widgetFactory->name();
    QPointer<QWidget> widget = m_Widgets.value(name);

    m_Widgets.remove(name);
    m_Factories.remove(name);

    if (!widget.isNull())
        delete widget;

    updateLayout();
}

void PanelImpl::updateLayout() {
    // Destroy layout;
    delete layout();

    QHBoxLayout * newLayout = new QHBoxLayout(this);
    
    QString order = module_settings->value("widgetsOrder", "MainMenuButton,QuickLaunchers,UserTasks,Indicators,DateTime").toString();
    QStringList widgets_Order = order.split(",");
    int i = 0;
    while (i < widgets_Order.size()) {
//        qDebug() << widgets_Order.at(i) << endl;
        QWidget* actualWidget = m_Widgets.value(widgets_Order.at(i)).data();
        if (actualWidget != NULL) {
            actualWidget->setGeometry(actualWidget->x(), actualWidget->y(), actualWidget->width(), this->height());
            newLayout->addWidget(actualWidget);
        }
        i++;
    }
    
    QMargins widgetsMargins(module_settings->value("buttonMarginLeft").toInt(), module_settings->value("buttonMarginTop").toInt(), 
                                module_settings->value("buttonMarginRight").toInt(), module_settings->value("buttonMarginBottom").toInt());
    newLayout->setContentsMargins(widgetsMargins);
    
    setLayout(newLayout);
}

int PanelImpl::desktop() {
    return m_Desktop;
}

void PanelImpl::setDesktop(int desktop) {
    KWindowSystem::setOnDesktop(windowHandle()->winId(), desktop);
    m_Desktop = desktop;
}

QRect PanelImpl::geometry() {
    return m_Geometry;
}

void PanelImpl::setGeometry(QRect geometry) {
    qWarning() << MODULE_NAME_STR << __PRETTY_FUNCTION__ << " not soported.";
}

/*Check if the widget's configuration is already set*/
bool PanelImpl::configExist(QString key) {
    
    if(module_settings->value(key) == 0)
        return false;
    else
        return true;
}
