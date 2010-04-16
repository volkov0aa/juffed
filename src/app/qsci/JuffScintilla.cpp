/*
JuffEd - An advanced text editor
Copyright 2007-2010 Mikhail Murzin

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License 
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "JuffScintilla.h"

#include "QSciSettings.h"

#include <QScrollBar>

#include <Qsci/qscicommandset.h>

//#include "CommandStorage.h"

#define WORD_HIGHLIGHT     1

JuffScintilla::JuffScintilla() : QsciScintilla() {
	rLine1_ = -1;
	rCol1_ = -1;
	rLine2_ = -1;
	rCol2_ = -1;

	initHighlightingStyle(WORD_HIGHLIGHT, QSciSettings::get(QSciSettings::WordHLColor));
	
	contextMenu_ = new QMenu();
/*	CommandStorage* st = CommandStorage::instance();
	contextMenu_->addAction(st->action(ID_EDIT_CUT));
	contextMenu_->addAction(st->action(ID_EDIT_COPY));
	contextMenu_->addAction(st->action(ID_EDIT_PASTE));
	contextMenu_->addSeparator();
	contextMenu_->addAction(st->action(ID_FIND));
	contextMenu_->addAction(st->action(ID_FIND_NEXT));
	contextMenu_->addAction(st->action(ID_FIND_PREV));
	contextMenu_->addAction(st->action(ID_REPLACE));
	contextMenu_->addSeparator();
	contextMenu_->addAction(st->action(ID_GOTO_LINE));*/
	
	connect(this, SIGNAL(linesChanged()), this, SLOT(updateLineNumbers()));
	
	// list of commands we want to disable initially
	QList<int> cmdsToRemove;
	cmdsToRemove << (Qt::Key_D | Qt::CTRL) << (Qt::Key_L | Qt::CTRL) 
	             << (Qt::Key_T | Qt::CTRL) << (Qt::Key_U | Qt::CTRL) 
	             << (Qt::Key_U | Qt::CTRL | Qt::SHIFT);
	
	QsciCommandSet* set = standardCommands();
	QList< QsciCommand*> cmds = set->commands();
	foreach (QsciCommand* cmd, cmds) {
		if ( cmdsToRemove.contains(cmd->key()) )
			cmd->setKey(0);
		if ( cmdsToRemove.contains(cmd->alternateKey()) )
			cmd->setAlternateKey(0);
	}
}

JuffScintilla::~JuffScintilla() {
	delete contextMenu_;
}

bool JuffScintilla::find(const Juff::SearchParams&) {
}

/*bool JuffScintilla::find(const QString& s, const DocFindFlags& flags) {
	if ( flags.multiLine )
		return findML(s, flags);

	QString str(s);
	QString text = this->text();
	QStringList lines = text.split(QRegExp("\r\n|\n|\r"));
	int row(-1), col(-1);
	this->getCursorPosition(&row, &col);
	int lineIndex(0);
	if (row < 0 || col < 0)
		return false;

	if ( !flags.backwards ) {
		foreach (QString line, lines) {
			if ( lineIndex < row ) {
			}
			else {
				int indent(0);
				if ( lineIndex == row ) {
					line = line.right(line.length() - col);
					indent = col;
				}
				int index(-1);
				QRegExp regExp;
				if ( flags.wholeWords ) {
					regExp = QRegExp(QString("\\b%1\\b").arg(QRegExp::escape(str)));
				}
				else
					regExp = QRegExp(str);
				regExp.setCaseSensitivity(flags.matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive);
				if ( flags.isRegExp || flags.wholeWords ) {
					index = line.indexOf(regExp);
				}
				else {
					if ( !flags.matchCase ) {
						str = str.toLower();
						line = line.toLower();
					}
					index = line.indexOf(str);
				}

				if ( index >= 0 ) {
					if ( flags.isRegExp ) {
						this->setSelection(lineIndex, index + indent, lineIndex, index + indent + regExp.matchedLength());
						this->ensureCursorVisible();
					}
					else {
						this->setSelection(lineIndex, index + indent, lineIndex, index + indent + str.length());
						this->ensureCursorVisible();
					}
					return true;
				}
			}
			++lineIndex;
		}
	}
	else {
		QStringList::iterator it = lines.end();
		it--;
		int lineIndex = lines.count() - 1;
		while ( lineIndex >= 0 ) {
			if ( lineIndex > row ) {
			}
			else {
				QString line = *it;
				if ( lineIndex == row ) {
					line = line.left(col);
				}

				int index(-1);
				QRegExp regExp;
				if ( flags.wholeWords )
					regExp = QRegExp(QString("\\b%1\\b").arg(QRegExp::escape(str)));
				else
					regExp = QRegExp(str);
				regExp.setCaseSensitivity(flags.matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive);
				if ( flags.isRegExp || flags.wholeWords )
					index = line.lastIndexOf(regExp);
				else {
					if ( !flags.matchCase ) {
						str = str.toLower();
						line = line.toLower();
					}
					index = line.lastIndexOf(str);
				}

				if ( index >= 0 ) {
					if ( flags.isRegExp ) {
						this->setSelection(lineIndex, index, lineIndex, index + regExp.matchedLength());
						this->ensureCursorVisible();
					}
					else {
						this->setSelection(lineIndex, index, lineIndex, index + str.length());
						this->ensureCursorVisible();
					}
					return true;
				}
			}
			lineIndex--;
			it--;
		}
	}

	return false;
}*/

void JuffScintilla::posToLineCol(long pos, int& line, int& col) const {
	line = SendScintilla(SCI_LINEFROMPOSITION, pos);
	long linpos = SendScintilla(SCI_POSITIONFROMLINE, line);
	col = (int)(pos - linpos);
}
/*
long JuffScintilla::lineColToPos(int line, int col) const {
	long linpos = SendScintilla(SCI_POSITIONFROMLINE, line);
	return linpos + col;
}

long JuffScintilla::curPos() const {
	int line, col;
	getCursorPosition(&line, &col);
	return lineColToPos(line, col);
}*/

QString JuffScintilla::wordUnderCursor() {
	int line, col;
	getCursorPosition(&line, &col);
	QString str = text(line);
	int startPos = str.left(col).lastIndexOf(QRegExp("\\b"));
	int endPos = str.indexOf(QRegExp("\\b"), col);
	if ( startPos >= 0 && endPos >= 0 && endPos > startPos )
		return str.mid(startPos, endPos - startPos);
	else
		return "";
}

/*bool JuffScintilla::findML(const QString& s, const DocFindFlags& flags) {
	JUFFENTRY;
	QString text = this->text();
	QRegExp rx(s);
	
	long pos = -1;
	if ( flags.backwards ) {
		long cPos;
		if ( hasSelectedText() ) {
			int line1, col1, line2, col2;
			getSelection(&line1, &col1, &line2, &col2);
			cPos = lineColToPos(line1, col1);
		}
		else {
			cPos = curPos();
		}

		pos = text.left(cPos).lastIndexOf(rx);
	}
	else {
		long cPos;
		if ( hasSelectedText() ) {
			int line1, col1, line2, col2;
			getSelection(&line1, &col1, &line2, &col2);
			cPos = lineColToPos(line2, col2);
		}
		else {
			cPos = curPos();
		}

		pos = text.indexOf(rx, cPos);
	}

	if ( pos >= 0 ) {
		int line1, col1, line2, col2;
		posToLineCol(pos, line1, col1);
		posToLineCol(pos + rx.matchedLength(), line2, col2);
		setSelection(line1, col1, line2, col2);
		ensureCursorVisible();
		return true;
	}
	else {
		return false;
	}
}

void JuffScintilla::replaceSelected(const QString& targetText, bool backwards) {
	beginUndoAction();
	removeSelectedText();
	int r, c;
	getCursorPosition(&r, &c);
	insert(targetText);
	if ( !backwards ) {
		setCursorPosition(r, c + targetText.length());
	}
	endUndoAction();
}*/

void JuffScintilla::dragEnterEvent(QDragEnterEvent* e) {
	if ( !e->mimeData()->hasUrls() )
		QsciScintilla::dragEnterEvent(e);
}

void JuffScintilla::dropEvent(QDropEvent* e) {
	if ( !e->mimeData()->hasUrls() )
		QsciScintilla::dropEvent(e);
}

void JuffScintilla::contextMenuEvent(QContextMenuEvent* e) {
	QPoint point = e->pos();
	
	/*
	* The following piece of code has been taken from 
	* QScintilla 2.3.2 source code in order to have this 
	* functionality in lower QScintilla versions 
	*/
	long position = SendScintilla(SCI_POSITIONFROMPOINTCLOSE, point.x(), point.y());

	int line = SendScintilla(SCI_LINEFROMPOSITION, position);
	int linpos = SendScintilla(SCI_POSITIONFROMLINE, line);
	int col = 0;

	// Allow for multi-byte characters.
	while ( linpos < position ) {
		int new_linpos = SendScintilla(SCI_POSITIONAFTER, linpos);

		// If the position hasn't moved then we must be at the end of the text
		// (which implies that the position passed was beyond the end of the
		// text).
		if ( new_linpos == linpos )
			break;

		linpos = new_linpos;
		++col;
	}
	/*
	* End of QScintilla code
	*/

	emit contextMenuCalled(line, col);
	contextMenu_->exec(e->globalPos());
}

void JuffScintilla::focusInEvent(QFocusEvent* e) {
	parentWidget()->setFocusProxy(this);
	QsciScintilla::focusInEvent(e);
	emit focusReceived();
}

void JuffScintilla::focusOutEvent(QFocusEvent* e) {
	cancelList();
	QsciScintilla::focusOutEvent(e);
}

void JuffScintilla::wheelEvent(QWheelEvent* e) {
	if ( e->modifiers() & Qt::ControlModifier ) {
		if ( e->delta() < 0 ) {
			zoomIn();
		}
		else if ( e->delta() > 0 ) {
			zoomOut();
		}
	}
	else {
		QsciScintilla::wheelEvent(e);
	}
}

void JuffScintilla::cancelRectInput() {
	rLine1_ = -1;
	rCol1_ = -1;
	rLine2_ = -1;
	rCol2_ = -1;
	SendScintilla(SCI_CANCEL);
}

void JuffScintilla::keyPressEvent(QKeyEvent* e) {
	if ( hasSelectedText() && SendScintilla(SCI_SELECTIONISRECTANGLE) ) {
		int line, col;
		getCursorPosition(&line, &col);

		int line1, col1, line2, col2;
		getSelection(&line1, &col1, &line2, &col2);
		rLine1_ = qMin(line1, line2);
		rCol1_ = qMin(col1, col2);
		rLine2_ = qMax(line1, line2);
		rCol2_ = qMax(col1, col2);

		switch ( e->key() ) {
			case Qt::Key_Escape :
				setSelection(line, col, line, col);
				cancelRectInput();
				break;

			case Qt::Key_Left:
			case Qt::Key_Right:
			case Qt::Key_Up:
			case Qt::Key_Down:
				if ( !(e->modifiers() & Qt::AltModifier) ) {
					setSelection(line, col, line, col);
					cancelRectInput();
				}
				else {
					QsciScintilla::keyPressEvent(e);
				}
				break;

			case Qt::Key_Backspace:
				if ( rCol1_ == rCol2_ )
					break;

				if ( col == rCol1_ ) {
					setSelection(rLine1_, rCol1_ , rLine2_, rCol2_);
					SendScintilla(SCI_SETSELECTIONMODE, 1);
				}
				
				beginUndoAction();
				//	select the last character of each line and remove it
				setSelection(rLine1_, rCol2_ - 1, rLine2_, rCol2_);
				SendScintilla(SCI_SETSELECTIONMODE, 1);
				removeSelectedText();
				
				--rCol2_;
				
				//	place new selection
				setSelection(rLine1_, rCol1_, rLine2_, rCol2_);
				SendScintilla(SCI_SETSELECTIONMODE, 1);
				endUndoAction();
				break;

			case Qt::Key_Delete :
				cancelRectInput();
				QsciScintilla::keyPressEvent(e);
				break;

			default:
				beginUndoAction();
				if ( rCol1_ != rCol2_ ) {
					//	TODO : probably need to remove selected text, but only once
				}
				QString t = e->text();
				for ( int i = line2; i >= line1; --i ) {
					insertAt(t, i, rCol2_ );
				}
				if ( e->key() != Qt::Key_Enter && e->key() != Qt::Key_Return ) {
					setSelection(rLine1_, rCol1_, rLine2_, rCol2_ + t.length());
					SendScintilla(SCI_SETSELECTIONMODE, 1);
				}
				endUndoAction();
		}
	}
	else {
		QsciScintilla::keyPressEvent(e);
	}
}

/*void JuffScintilla::addContextMenuActions(const ActionList& list) {
	foreach (QAction* a, list) {
		contextMenu_->addAction(a);
	}
}
*/
void JuffScintilla::showLineNumbers(bool show) {
	showLineNumbers_ = show;
	updateLineNumbers();
}

bool JuffScintilla::lineNumbersVisible() const {
	return showLineNumbers_;
}

void JuffScintilla::updateLineNumbers() {
	if ( showLineNumbers_ ) {
		QString str = QString("00%1").arg(lines());
		setMarginWidth(1, str);
	}
	else {
		setMarginWidth(1, 0);
	}
}

void JuffScintilla::clearHighlighting() {
	SendScintilla(SCI_SETINDICATORCURRENT, WORD_HIGHLIGHT);
	SendScintilla(SCI_INDICATORCLEARRANGE, 0, length());
}

void JuffScintilla::highlightText(const QString& text) {
	if ( hasSelectedText() ) {
		return;
	}
	
	clearHighlighting();
	
	if ( text.length() < 2 )
		return;
	
	int initialLine, initialCol;
	getCursorPosition(&initialLine, &initialCol);
	int scrollPos = verticalScrollBar()->value();
	
	int line = 0, col = 0;
	while ( findFirst(text, false, false, true, false, true, line, col) ) {
		int start = SendScintilla(SCI_GETSELECTIONSTART);
		int end = SendScintilla(SCI_GETSELECTIONEND);
		highlight(start, end, WORD_HIGHLIGHT);
		posToLineCol(end, line, col);
	}
	
	setCursorPosition(initialLine, initialCol);
	verticalScrollBar()->setValue(scrollPos);
}

void JuffScintilla::highlight(int start, int end, int ind) {
	SendScintilla(SCI_SETINDICATORCURRENT, ind);
	SendScintilla(SCI_INDICATORFILLRANGE, start, end - start);
}

void JuffScintilla::initHighlightingStyle(int id, const QColor &color) {
	SendScintilla(SCI_INDICSETSTYLE, id, INDIC_ROUNDBOX);
	SendScintilla(SCI_INDICSETUNDER, id, true);
	SendScintilla(SCI_INDICSETFORE, id, color);
	SendScintilla(SCI_INDICSETALPHA, id, 50);
}