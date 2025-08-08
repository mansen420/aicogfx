#pragma once

#include "aico/opres.h"

namespace aico::sys
{
   /**
     * @brief Initializes the engine.
     * This must be called before any other engine construct may be used.
     * Calling this after the engine has already been initialized will have no effect.
     * This may be used to re-initialize the engine after a call to aicogfx::terminate()
     * Engine initialization status is marked in aicogfx::engine_flags with the 
     * aicogfx::engine_flag_bits::INIT bit.
     *
     * @return aicogfx::opres::SUCCESS for success, 
     * consult return status codes otherwise.
     */
     opres init();
    /**
     * @brief Terminates engine resources. 
     * Calling this before the engine has been initialized has no effect.
     * No engine constructs may be used after calling this.
     */
    void terminate()noexcept;
}
