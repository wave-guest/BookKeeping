#include "PageController.h"

#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>

class PageController::Impl
{
public:
    QPushButton* firstBtn;
    QPushButton* prevBtn;
    QPushButton* nextBtn;
    QPushButton* lastBtn;
    QLabel* pageLabel;

    int currentPage = 1;
    int totalPage = 1;
    int pageSize = 20;

    void updateUI()
    {
        pageLabel->setText(QStringLiteral("\u7b2c %1/%2 \u9875").arg(currentPage).arg(totalPage));
        firstBtn->setEnabled(currentPage > 1);
        prevBtn->setEnabled(currentPage > 1);
        nextBtn->setEnabled(currentPage < totalPage);
        lastBtn->setEnabled(currentPage < totalPage);
    }
};

PageController::PageController(QWidget* parent)
    : QWidget(parent)
{
    m_pImpl = std::make_unique<Impl>();

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);

    m_pImpl->firstBtn = new QPushButton(QStringLiteral("\u9996\u9875"), this);
    m_pImpl->prevBtn = new QPushButton(QStringLiteral("\u4e0a\u4e00\u9875"), this);
    m_pImpl->pageLabel = new QLabel(QStringLiteral("\u7b2c 1/1 \u9875"), this);
    m_pImpl->nextBtn = new QPushButton(QStringLiteral("\u4e0b\u4e00\u9875"), this);
    m_pImpl->lastBtn = new QPushButton(QStringLiteral("\u672b\u9875"), this);

    layout->addStretch();
    layout->addWidget(m_pImpl->firstBtn);
    layout->addWidget(m_pImpl->prevBtn);
    layout->addWidget(m_pImpl->pageLabel);
    layout->addWidget(m_pImpl->nextBtn);
    layout->addWidget(m_pImpl->lastBtn);
    layout->addStretch();

    connect(m_pImpl->firstBtn, &QPushButton::clicked, this, &PageController::FirstPage);
    connect(m_pImpl->prevBtn, &QPushButton::clicked, this, &PageController::PrevPage);
    connect(m_pImpl->nextBtn, &QPushButton::clicked, this, &PageController::NextPage);
    connect(m_pImpl->lastBtn, &QPushButton::clicked, this, &PageController::LastPage);

    m_pImpl->updateUI();
}

PageController::~PageController() = default;

void PageController::setCurrentPage(int page)
{
    m_pImpl->currentPage = qBound(1, page, m_pImpl->totalPage);
    m_pImpl->updateUI();
}

void PageController::setTotalPage(int total, int pageSize)
{
    m_pImpl->pageSize = pageSize;
    m_pImpl->totalPage = (total + pageSize - 1) / pageSize;
    if (m_pImpl->totalPage < 1) m_pImpl->totalPage = 1;
    m_pImpl->currentPage = qBound(1, m_pImpl->currentPage, m_pImpl->totalPage);
    m_pImpl->updateUI();
}

void PageController::FirstPage()
{
    m_pImpl->currentPage = 1;
    m_pImpl->updateUI();
    emit onFirstPage();
    emit pageChanged(m_pImpl->currentPage);
}

void PageController::PrevPage()
{
    if (m_pImpl->currentPage > 1)
    {
        --m_pImpl->currentPage;
        m_pImpl->updateUI();
        emit onPrevPage();
        emit pageChanged(m_pImpl->currentPage);
    }
}

void PageController::NextPage()
{
    if (m_pImpl->currentPage < m_pImpl->totalPage)
    {
        ++m_pImpl->currentPage;
        m_pImpl->updateUI();
        emit onNextPage();
        emit pageChanged(m_pImpl->currentPage);
    }
}

void PageController::LastPage()
{
    m_pImpl->currentPage = m_pImpl->totalPage;
    m_pImpl->updateUI();
    emit onLastPage();
    emit pageChanged(m_pImpl->currentPage);
}