#pragma once

class ITask {
    public:
        virtual ~ITask() = default;

        virtual void execute() = 0;

    private:
        ITask();
};