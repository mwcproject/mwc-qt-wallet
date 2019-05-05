#include "state/state.h"

namespace state {

State::State(const StateContext & _context, STATE _stateId) :
    context(_context), stateId(_stateId)
{}

State::~State() {}

}
