#ifndef INTSPINBOX_H
#define INTSPINBOX_H

#include <QSpinBox>
#include <QTimerEvent>
#include <QWidget>
#include <QAbstractSpinBox>
#include <QLineEdit>

// https://bugreports.qt.io/browse/QTBUG-14259
class IntSpinBox : public QSpinBox
{
	public:
		explicit IntSpinBox(QWidget* parent = nullptr);
		
		void stepBy(int steps) override;
		
		//protected:
		
		//void timerEvent(QTimerEvent *event) override;
	
};


#endif // INTSPINBOX_H