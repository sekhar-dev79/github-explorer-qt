#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QIcon>

class WelcomePage : public QWidget
{
    Q_OBJECT

public:
    explicit WelcomePage(QWidget* parent = nullptr);

signals:
    void startExploringRequested();

private:
    void setupUI();
    QIcon loadIcon(const QString& iconName);

    QLabel* m_titleLabel    = nullptr;
    QLabel* m_subtitleLabel = nullptr;
    QPushButton* m_ctaButton     = nullptr;
};
