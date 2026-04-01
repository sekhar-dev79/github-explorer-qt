#pragma once

#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>
#include <QTimer>
#include <QHBoxLayout>

class Toast : public QWidget
{
    Q_OBJECT
public:
    // Static helper to easily call the toast from anywhere
    static void show(QWidget* parent, const QString& message, const QString& iconName = "check-circle");

private:
    explicit Toast(QWidget* parent, const QString& message, const QString& iconName);
    void startAnimation();

    QLabel* m_iconLabel;
    QLabel* m_textLabel;
    QPropertyAnimation* m_opacityAnimation;
    QTimer* m_timer;
};
