#include "IntSpinBox.hpp"


IntSpinBox::IntSpinBox(QWidget* parent) : QSpinBox(parent) { }

/* Override default behavior of stepping if step buttons are held down
void IntSpinBox::timerEvent(QTimerEvent *event){
	event->accept();
	QWidget::timerEvent(event); 
}*/

// Do not highlight the text in the spinbox after step buttons are pressed
void IntSpinBox::stepBy(int steps){
/* COMMENTO: vogliamo tenerci una sottoclassse di QSpinBox solo per questo? */
	QSpinBox::stepBy(steps);
	QAbstractSpinBox::lineEdit()->deselect();
}