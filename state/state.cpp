#include "state.h"
#include  "../core/mainwindow.h"

namespace state {

State::State(StateContext * _context, STATE _stateId) :
    context(_context), stateId(_stateId)
{}

State::~State() {}

void State::setWindowTitle( QString title ) {
    context->mainWnd->setWindowTitle(title);
}

}
