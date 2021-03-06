/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "workspace.h"

#include <QWidget>

namespace Ui
{
class ElementEditor;
}

class ElementEditor : public QWidget
{
    Q_OBJECT

public:
    explicit ElementEditor(QWidget *parent = nullptr);
    ~ElementEditor() override;

    bool eventFilter(QObject *obj, QEvent *event) override;
    void changeTriggerAction();
    void contextMenu(QPoint screenPos);
    void disable();
    void fillColorComboBox();
    void renameAction();
    void retranslateUi();
    void setScene(Scene *scene);
    void updateElementSkin();

signals:
    void sendCommand(QUndoCommand *cmd);

private:
    void apply();
    void defaultSkin();
    void inputIndexChanged(const int index);
    void inputLocked(const bool value);
    void outputIndexChanged(const QString &index);
    void outputValueChanged(const QString &index);
    void selectionChanged();
    void setCurrentElements(const QVector<GraphicElement *> &elms);
    void triggerChanged(const QString &cmd);
    void updateSkins();

    Ui::ElementEditor *m_ui;
    QString m_manyAudios = tr("<Many sounds>");
    QString m_manyColors = tr("<Many colors>");
    QString m_manyFreq = tr("<Many values>");
    QString m_manyIS = tr("<Many values>");
    QString m_manyLabels = tr("<Many labels>");
    QString m_manyOS = tr("<Many values>");
    QString m_manyOV = tr("<Many values>");
    QString m_manyTriggers = tr("<Many triggers>");
    QString m_skinName;
    QVector<GraphicElement *> m_elements;
    Scene *m_scene = nullptr;
    bool m_canChangeInputSize = false;
    bool m_canChangeOutputSize = false;
    bool m_canChangeSkin = false;
    bool m_canMorph = false;
    bool m_hasAnyProperty = false;
    bool m_hasAudio = false;
    bool m_hasColors = false;
    bool m_hasElements = false;
    bool m_hasFrequency = false;
    bool m_hasLabel = false;
    bool m_hasOnlyInputs = false;
    bool m_hasRotation = false;
    bool m_hasSameAudio = false;
    bool m_hasSameColors = false;
    bool m_hasSameFrequency = false;
    bool m_hasSameInputSize = false;
    bool m_hasSameLabel = false;
    bool m_hasSameOutputSize = false;
    bool m_hasSameOutputValue = false;
    bool m_hasSameTrigger = false;
    bool m_hasSameType = false;
    bool m_hasTrigger = false;
    bool m_isDefaultSkin = true;
    bool m_isUpdatingSkin = false;
};

