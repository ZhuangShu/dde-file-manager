/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "dfmaddressbar.h"

#include "views/dcompleterlistview.h"

#include "themeconfig.h"

#include <QAction>
#include <QCompleter>
#include <QAbstractItemView>
#include <QTimer>
#include <QPainter>
#include <QScrollBar>
#include <QDebug>

#include <DThemeManager>

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

DCompleterStyledItemDelegate::DCompleterStyledItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void DCompleterStyledItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // prepare;
    QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
            ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(option.state & QStyle::State_Active)) {
        cg = QPalette::Inactive;
    }

    // draw background
    if (option.showDecorationSelected && (option.state & (QStyle::State_Selected | QStyle::State_MouseOver))) {
        painter->fillRect(option.rect, option.palette.brush(cg, QPalette::Highlight));
    }

    // draw text
    if (option.state & (QStyle::State_Selected | QStyle::State_MouseOver)) {
        painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(cg, QPalette::Text));
    }

    painter->setFont(option.font);
    painter->drawText(option.rect.adjusted(31, 0, 0, 0), Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());
}

QSize DCompleterStyledItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize s = QStyledItemDelegate::sizeHint(option, index);
    s.setHeight(24);

    return s;
}

DFMAddressBar::DFMAddressBar(QWidget *parent)
    : QLineEdit(parent)
{
    initUI();
    initConnections();

    // Test
    urlCompleter = new QCompleter(this);
    this->setCompleter(urlCompleter);
    completerModel.setStringList({"macat", "mamacat", "mamamacat", "mamamamacat", "mamamamamacat",
                                  "makat", "mamakat", "mamamakat", "mamamamakat", "mamamamamakat",
                                  "mafox", "teji", "tejilang", "jjy", "jjjjy"});
}

QCompleter *DFMAddressBar::completer() const
{
    return urlCompleter;
}

void DFMAddressBar::setCurrentUrl(const DUrl &path)
{
    QString text = path.isLocalFile() ? path.toLocalFile() : path.toString();

    currentUrl = path;
    //m_disableCompletion = true;
    this->setText(text);
    this->setSelection(0, text.length());
}

void DFMAddressBar::setCompleter(QCompleter *c)
{
    if (urlCompleter) {
        urlCompleter->disconnect();
    }

    urlCompleter = c;

    if (!urlCompleter) {
        return;
    }

    urlCompleter->setModel(&completerModel);
    urlCompleter->setPopup(completerView);
    urlCompleter->setCompletionMode(QCompleter::PopupCompletion);
    urlCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    urlCompleter->setMaxVisibleItems(10);
    connect(urlCompleter, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));
    connect(urlCompleter, SIGNAL(highlighted(QString)), this, SLOT(onCompletionHighlighted(QString)));

    completerView->setItemDelegate(&styledItemDelegate);

}

void DFMAddressBar::focusInEvent(QFocusEvent *e)
{
    if (urlCompleter) {
        urlCompleter->setWidget(this);
    }

    return QLineEdit::focusInEvent(e);
}

void DFMAddressBar::focusOutEvent(QFocusEvent *e)
{
    emit focusOut();

    return QLineEdit::focusOutEvent(e);
}

void DFMAddressBar::keyPressEvent(QKeyEvent *e)
{
    if (urlCompleter && urlCompleter->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
        switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
        default:
           break;
        }
    }

    if (!urlCompleter || !false/*isShortcut*/) {
        QLineEdit::keyPressEvent(e);
    }

    if (!urlCompleter) {
        return;
    }

    if (text().isEmpty()) {
        urlCompleter->popup()->hide();
        return;
    }

    // blumia: Assume address is: /aa/bbbb/cc , completion prefix should be "cc",
    //         completerBaseString should be "/aa/bbbb/"
    urlCompleter->setCompletionPrefix(this->text());
    this->completerBaseString = "";
    urlCompleter->complete(rect().adjusted(0, 5, 0, 5));
}

void DFMAddressBar::initUI()
{
    // Left indicator (clickable! did u know that?)
    indicator = new QAction(this);
    addAction(indicator, QLineEdit::LeadingPosition);

    // Clear text button
    setClearButtonEnabled(true);

    // Completer List
    completerView = new DCompleterListView(this);

    // Other misc..
    setFixedHeight(24);
    setObjectName("DSearchBar");
    setMinimumWidth(1);
    setAlignment(Qt::AlignHCenter);

    setFocusPolicy(Qt::ClickFocus);
}

void DFMAddressBar::initConnections()
{
    connect(DThemeManager::instance(), &DThemeManager::widgetThemeChanged, this, &DFMAddressBar::onWidgetThemeChanged);
    connect(indicator, &QAction::triggered, this, [this](){
        emit returnPressed();
    });
}

void DFMAddressBar::setIndicator(DFMAddressBar::IndicatorType type)
{
    indicatorType = type;
    updateIndicatorIcon();
}

void DFMAddressBar::onWidgetThemeChanged(QWidget *widget, QString theme)
{
    DThemeManager::instance()->setTheme(completerView, theme);

    if (widget == this) {
        updateIndicatorIcon();
    }
}

void DFMAddressBar::updateIndicatorIcon()
{
    QIcon indicatorIcon;
    QString scope = indicatorType == IndicatorType::Search ? "DSearchBar.searchAction" : "DSearchBar.jumpToAction";
    indicatorIcon.addFile(ThemeConfig::instace()->value(scope, "icon").toString());
    indicator->setIcon(indicatorIcon);
}

void DFMAddressBar::insertCompletion(const QString &completion)
{
    if (urlCompleter->widget() != this) {
        return;
    }

    this->setText(completerBaseString + completion);
}

void DFMAddressBar::onCompletionHighlighted(const QString &highlightedCompletion)
{
    int completionPrefixLen = urlCompleter->completionPrefix().length();
    QString shouldAppend = highlightedCompletion.right(highlightedCompletion.length() - completionPrefixLen);
    setText(completerBaseString + urlCompleter->completionPrefix() + shouldAppend);
    setSelection(text().length() - shouldAppend.length(), text().length());
}

DFM_END_NAMESPACE
