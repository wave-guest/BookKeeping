#include "PageController.h"

#include <QPushButton>
#include <QLabel>

class PageController::Impl
{
public:
	QPushButton* firstBtn;
	QPushButton* prevBtn;
	QPushButton* nextBtn;
	QPushButton* lastBtn;
	QLabel* pageLabel;

	int currentPage = 0;
	int totalPage = 0;
	int pageSize = 20;

private: 
	void updateStatus();
};

PageController::PageController(QWidget* parent)
{
	m_pImpl = std::make_unique<Impl>();

	m_pImpl->firstBtn = new QPushButton("首页", this);
	m_pImpl->prevBtn = new QPushButton("上一页", this);
	m_pImpl->nextBtn = new QPushButton("下一页", this);
	m_pImpl->lastBtn = new QPushButton("末页", this);
	m_pImpl->pageLabel = new QLabel("第 0/0 页", this);

	connect(m_pImpl->firstBtn, &QPushButton::clicked, this, &PageController::FirstPage);
	connect(m_pImpl->prevBtn, &QPushButton::clicked, this, &PageController::PrevPage);
	connect(m_pImpl->nextBtn, &QPushButton::clicked, this, &PageController::NextPage);
	connect(m_pImpl->lastBtn, &QPushButton::clicked, this, &PageController::LastPage);
}

PageController::~PageController()
{

}

void PageController::setCurrentPage(int page)
{
	m_pImpl->currentPage = page;
}

void PageController::setTotalPage(int total, int pageSize)
{
	m_pImpl->pageSize = pageSize;
	m_pImpl->totalPage = (total + pageSize - 1)/pageSize;

}


void PageController::FirstPage()
{
	if (m_pImpl->currentPage <= 1)
	{
		m_pImpl->firstBtn->setDisabled(true);
		return;
	}
}
void PageController::PrevPage()
{
	if (m_pImpl->currentPage <= 1)
	{
		m_pImpl->firstBtn->setDisabled(true);
		return;
	}
}
void PageController::NextPage()
{

}
void PageController::LastPage()
{

}