#include "IntSpinBox.hpp"


IntSpinBox::IntSpinBox(QWidget* parent) : QSpinBox(parent) { }

// Override default behavior of stepping if step buttons are held down
//void IntSpinBox::timerEvent(QTimerEvent *event){
	//event->accept();
	//QWidget::timerEvent(event); // NOLINT(bugprone-parent-virtual-call)
//}

// Do not highlight the text in the spinbox after step buttons are pressed
void IntSpinBox::stepBy(int steps){
	QSpinBox::stepBy(steps);
	QAbstractSpinBox::lineEdit()->deselect();
}