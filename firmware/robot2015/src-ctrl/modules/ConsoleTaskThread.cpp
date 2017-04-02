#include "Assert.hpp"
#include "Commands.hpp"
#include "Console.hpp"
#include "Logger.hpp"
#include "Rtos.hpp"
#include "TaskSignals.hpp"
#include "git_version.hpp"
#include "mbed_rpc.h"

/**
 * Initializes the console
 */
void Task_SerialConsole(const void* args) {
    const auto mainID = static_cast<osThreadId>(const_cast<void*>(args));

    // Store the thread's ID
    const auto threadID = Thread::gettid();
    ASSERT(threadID != nullptr);

    // Store our priority so we know what to reset it to after running a command
    const auto threadPriority = osThreadGetPriority(threadID);

    // Initalize the console buffer and save the char buffer's starting address
    // std::shared_ptr<Console> console = ;
    Console::Instance = make_shared<Console>();

    // Set the console username to whoever the git author is
    Console::Instance->changeUser(git_head_author);

    // Let everyone know we're ok
    LOG(OK,
        "Serial console ready!\r\n"
        "    Thread ID: %u, Priority: %d",
        reinterpret_cast<P_TCB>(threadID)->task_id, threadPriority);

    // Signal back to main and wait until we're signaled to continue
    osSignalSet(mainID, MAIN_TASK_CONTINUE);
    Thread::signal_wait(SUB_TASK_CONTINUE, osWaitForever);

    // Display RoboJackets if we're up and running at this point during startup
    Console::Instance->ShowLogo();

    // Print out the header to show the user we're ready for input
    Console::Instance->PrintHeader();

    // Set the title of the terminal window
    Console::Instance->SetTitle("RoboJackets");

    while (true) {
        // Execute any active iterative command
        execute_iterative_command();

        // If there is a new command to handle, parse and process it
        if (Console::Instance->CommandReady() == true) {
// Increase the thread's priority first so we can make sure the
// scheduler will select it to run
#ifndef NDEBUG
            auto tState = osThreadSetPriority(threadID, osPriorityAboveNormal);
            ASSERT(tState == osOK);
#else
            osThreadSetPriority(threadID, osPriorityAboveNormal);
#endif

            // Execute the command
            const auto rxLen = Console::Instance->rxBuffer().size() + 1;
            char rx[rxLen];
            memcpy(rx, Console::Instance->rxBuffer().c_str(), rxLen - 1);
            rx[rxLen - 1] = '\0';

            // Detach the console from reading stdin while the comamnd is
            // running to allow the command to read input.  We re-attach the
            // Console's handler as soon as the command is done executing.
            Console::Instance->detachInputHandler();
            execute_line(rx);
            // flush any extra characters that were input while executing cmd
            while (Console::Instance->pc.readable())
                Console::Instance->pc.getc();
            Console::Instance->attachInputHandler();

// Now, reset the priority of the thread to its idle state
#ifndef NDEBUG
            tState = osThreadSetPriority(threadID, threadPriority);
            ASSERT(tState == osOK);
#else
            osThreadSetPriority(threadID, threadPriority);
#endif

            Console::Instance->CommandHandled();
        }

        // Check if a system stop is requested
        if (Console::Instance->IsSystemStopRequested() == true) break;

        // Yield to other threads when not needing to execute anything
        Thread::yield();
    }

    ASSERT(!"Execution is at an unreachable line!");
}
