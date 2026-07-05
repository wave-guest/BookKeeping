#pragma once 

#include <memory>
#include <QWidget>

#include "Export.h"

class MYWINDOW_EXPORT PageController : public QWidget
{
	Q_OBJECT
public:
	explicit PageController(QWidget* parent = nullptr);
	~PageController();

	void setCurrentPage(int page);
	void setTotalPage(int total, int pageSize = 20);

signals:
	void pageChanged(int page);

private slots:
	void FirstPage();
	void PrevPage();
	void NextPage();
	void LastPage();

private:
	class Impl;
	std::unique_ptr<Impl> m_pImpl;
};