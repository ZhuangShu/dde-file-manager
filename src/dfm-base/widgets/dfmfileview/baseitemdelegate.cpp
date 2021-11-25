/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "baseitemdelegate.h"
#include "private/baseitemdelegate_p.h"
#include "widgets/dfmfileview/fileview.h"

DFMBASE_BEGIN_NAMESPACE

BaseItemDelegate::BaseItemDelegate(FileView *parent)
    : QStyledItemDelegate(parent),
      d(new BaseItemDelegatePrivate(this))
{
    Q_D(BaseItemDelegate);
    d->fileView = parent;
    d->textLineHeight = parent->fontMetrics().lineSpacing();
}

BaseItemDelegate::BaseItemDelegate(BaseItemDelegatePrivate &dd, FileView *parent)
    : QStyledItemDelegate(parent),
      d(&dd)
{
    Q_D(BaseItemDelegate);
    d->fileView = parent;
    d->textLineHeight = parent->fontMetrics().lineSpacing();
}

BaseItemDelegate::~BaseItemDelegate()
{
}

int BaseItemDelegate::setIconSizeByIconSizeLevel(int level)
{
    Q_UNUSED(level)
    return -1;
}

DFMBASE_END_NAMESPACE
