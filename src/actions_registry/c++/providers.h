/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXDE-Qt - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2010-2011 Razor team
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *   Petr Vanek <petr@scribus.info>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */


#ifndef PROVIDERS_H
#define PROVIDERS_H

#include "actions_registry/ICommandProvider.h"

#include <QtCore/QList>
#include <QtCore/QRegExp>
#include <QtXml/QDomElement>
#include <QtCore/QString>
#include <QtGui/QIcon>

#ifdef HAVE_MENU_CACHE
#include <menu-cache/menu-cache.h>
#endif


/************************************************
 * Application desktop files
 ************************************************/

class AppLinkItem: public CommandProviderItem
{
    Q_OBJECT
public:
    AppLinkItem(const QDomElement &element);

#ifdef HAVE_MENU_CACHE
    AppLinkItem(MenuCacheApp* app);
#endif

    bool run() const;
    bool compare(const QRegExp &regExp) const;
    QString command() const { return mCommand; }

    void operator=(const AppLinkItem &other);

    virtual unsigned int rank(const QString &pattern) const;
private slots:
    void updateIcon();
private:
    QString mDesktopFile;
    QString mIconName;
    QString mCommand;
    QString mProgram;
};


class XdgMenu;
class AppLinkProvider: public ICommandProvider
{
    Q_OBJECT
public:
    AppLinkProvider();
    virtual ~AppLinkProvider();

private slots:
    void update();

private:
#ifdef HAVE_MENU_CACHE
    MenuCache* mMenuCache;
    MenuCacheNotifyId mMenuCacheNotify;
    static void menuCacheReloadNotify(MenuCache* cache, gpointer user_data);
#else
    XdgMenu *mXdgMenu;
#endif
};


/************************************************
 * History
 ************************************************/

class HistoryItem: public CommandProviderItem
{
public:
    HistoryItem(const QString &command);

    bool run() const;
    bool compare(const QRegExp &regExp) const;

    QString command() const { return mCommand; }
    virtual unsigned int rank(const QString &pattern) const;

private:
    QString mCommand;
};



class QSettings;
class HistoryProvider: public ICommandProvider
{
public:
    HistoryProvider();
    virtual ~HistoryProvider();

    void AddCommand(const QString &command);
    void clearHistory();

private:
    QSettings *mHistoryFile;
};



/************************************************
 * Custom command
 ************************************************/
class CustomCommandProvider;

class CustomCommandItem: public CommandProviderItem
{
public:
    CustomCommandItem(CustomCommandProvider *provider);

    bool run() const;
    bool compare(const QRegExp &regExp) const;

    QString command() const { return mCommand; }
    void setCommand(const QString &command);

    virtual unsigned int rank(const QString &pattern) const;
private:
    QString mCommand;
    CustomCommandProvider *mProvider;
};



class QSettings;
class CustomCommandProvider: public ICommandProvider
{
public:
    CustomCommandProvider();

    QString command() const { return mItem->command(); }
    void setCommand(const QString &command) { mItem->setCommand(command); }

    HistoryProvider* historyProvider() const { return mHistoryProvider; }
    void setHistoryProvider(HistoryProvider *historyProvider) { mHistoryProvider = historyProvider; }

private:
    CustomCommandItem *mItem;
    HistoryProvider *mHistoryProvider;
};



/************************************************
 * Mathematics
 ************************************************/
class MathItem: public CommandProviderItem
{
public:
    MathItem();

    bool run() const;
    bool compare(const QRegExp &regExp) const;
    virtual unsigned int rank(const QString &pattern) const;
};



class MathProvider: public ICommandProvider
{
public:
    MathProvider();
    //virtual ~MathProvider();
};

#ifdef VBOX_ENABLED
#include <QtCore/QDateTime>
#include <QtGui/QDesktopServices>
#include <QtCore/QFileInfo>
#include <QtCore/QMap>

class VirtualBoxItem: public CommandProviderItem
{
public:
  VirtualBoxItem(const QString & MachineName , const QIcon & Icon);
  
  void setRDEPort (const QString & portNum);
  bool run() const;
  bool compare(const QRegExp &regExp) const;
  virtual unsigned int rank(const QString &pattern) const;
private:
  QString m_rdePortNum;
};

class VirtualBoxProvider: public ICommandProvider
{
public:
  VirtualBoxProvider ();
  void rebuild();
  bool isOutDated() const;
  
private:
  QFile fp;
  QMap<QString,QString> osIcons;
  QString virtualBoxConfig;
  QDateTime timeStamp;
};
#endif


class QAction;
/*! Power management built in into runner
 */
class PowerProviderItem : public CommandProviderItem
{
public:
    PowerProviderItem(QAction *action);

    bool run() const;
    bool compare(const QRegExp &regExp) const;
    unsigned int rank(const QString &pattern) const;
private:
    QAction *m_action;
};

namespace LxQt {
    class PowerManager;
    class ScreenSaver;
}

/*! Power management built in into runner
 */
class PowerProvider: public ICommandProvider
{
    Q_OBJECT
public:
    PowerProvider();

private:
    LxQt::PowerManager *m_power;
    LxQt::ScreenSaver *m_screensaver;
};

/*
class ExternalProviderItem: public CommandProviderItem
{
    Q_OBJECT

public:
    ExternalProviderItem();

    bool setData(QMap<QString, QString> & data);

    bool run() const;
    bool compare(const QRegExp &regExp) const {return true;} // We leave the decision to the external process
    unsigned int rank(const QString &pattern) const; 

    QString mCommand;
};

class QProcess;
class YamlParser;
class ExternalProvider: public ICommandProvider
{
    Q_OBJECT

public:
    ExternalProvider(const QString name, const QString externalProgram);
  
    void setSearchTerm(const QString searchTerm);
    
signals:
    void dataChanged();

private slots:
    void readFromProcess();
    void newListOfMaps(QList<QMap<QString, QString> > maps);
    
private:
    QString mName; 
    QProcess *mExternalProcess;
    QTextStream *mDataToProcess; 
    YamlParser *mYamlParser; 

    QByteArray mBuffer;
};
*/

#endif // PROVIDERS_H
