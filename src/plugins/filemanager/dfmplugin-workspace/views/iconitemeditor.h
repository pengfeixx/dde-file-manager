/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhangsheng<zhangsheng@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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
#ifndef ICONITEMEDITOR_H
#define ICONITEMEDITOR_H

#include "dfmplugin_workspace_global.h"

#include <QLabel>
#include <QStack>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE
class QTextEdit;
class QGraphicsOpacityEffect;
QT_END_NAMESPACE

DPWORKSPACE_BEGIN_NAMESPACE

class IconItemEditorPrivate;

class IconItemEditor : public QFrame
{
    Q_OBJECT

    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    explicit IconItemEditor(QWidget *parent = nullptr);
    ~IconItemEditor() override;

    qreal opacity() const;
    void setOpacity(qreal opacity);
    void setMaxCharSize(int maxSize);
    int maxCharSize() const;

    QSize sizeHint() const override;

    QLabel *getIconLabel() const;

    QTextEdit *getTextEdit() const;

public slots:

    /**
     * @brief showAlertMessage 显示提示信息
     * @param text 提示内容
     * @param duration 显示时间，默认3秒
     */
    void showAlertMessage(const QString &text, int duration = 3000);

signals:
    void inputFocusOut();

private slots:
    void popupEditContentMenu();
    void editUndo();
    void editRedo();
    void doLineEditTextChanged();
    void resizeFromEditTextChanged();

protected:
    void updateEditorGeometry();
    bool event(QEvent *ee) override;
    bool eventFilter(QObject *obj, QEvent *ee) override;

private:
    void updateStyleSheet();

    QString editTextStackCurrentItem() const;
    QString editTextStackBack();
    QString editTextStackAdvance();
    void pushItemToEditTextStack(const QString &item);

    friend class IconItemDelegate;
    friend class FileView;
    friend class FileViewHelper;

    QScopedPointer<IconItemEditorPrivate> dd;

    Q_DECLARE_PRIVATE_D(dd, IconItemEditor)
    Q_DISABLE_COPY(IconItemEditor)
};
DPWORKSPACE_END_NAMESPACE

#endif   // ICONITEMEDITOR_H