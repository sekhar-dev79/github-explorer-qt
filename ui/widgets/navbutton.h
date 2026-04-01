#pragma once

#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QEnterEvent>
#include <QEvent>

class NavButton : public QPushButton
{
    Q_OBJECT

public:
    explicit NavButton(const QString& iconName,
                       const QString& text,
                       QWidget* parent = nullptr);

    void setNavText(const QString& text);
    [[nodiscard]] QString navText() const;

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private slots:
    void updateIconColor();

private:
    void buildLayout(const QString& text);

    QString m_iconName;
    QLabel* m_iconLabel = nullptr;
    QLabel* m_textLabel = nullptr;
};
