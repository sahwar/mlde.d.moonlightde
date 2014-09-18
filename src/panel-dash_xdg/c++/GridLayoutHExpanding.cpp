/*
 * Copyright (C) 2014 Moonlight Desktop Environment Team
 * Authors:
 * Ruben S. Garcia San Juan
 * 
 * This file is part of Moonlight Desktop Environment Core.
 *
 * Moonlight Desktop Environment is free software: you can redistribute it 
 * and/or modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Moonlight Desktop Environment is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Moonlight Desktop Environment. If not, see <http://www.gnu.org/licenses/>.
 */

#include "GridLayoutHExpanding.h"

GridLayoutHExpanding::GridLayoutHExpanding(int cMaxColumnsApp, QWidget* parent) : QGridLayout(parent) {
    maxRow = cMaxColumnsApp;
    currentColumn = 0;
    currentRow = 0;
}

GridLayoutHExpanding::GridLayoutHExpanding(const GridLayoutHExpanding& orig) {

}

GridLayoutHExpanding::~GridLayoutHExpanding() {

}

void GridLayoutHExpanding::addWidget(QWidget* widget) {

    if (currentRow == maxRow) {
        currentColumn++;
        currentRow = 0;
    }

    QGridLayout::addWidget(widget, currentRow, currentColumn, Qt::AlignCenter);
    currentRow++;
}

void GridLayoutHExpanding::addLayout(QLayout* layout) {

    if (currentRow == maxRow) {
        currentColumn++;
        currentRow = 0;
    }

    QGridLayout::addLayout(layout, currentRow, currentColumn, Qt::AlignCenter);
    currentRow++;
}

