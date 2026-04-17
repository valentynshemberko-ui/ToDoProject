#ifndef ITASKOBSERVER_H
#define ITASKOBSERVER_H

class ITaskObserver {
public:
    virtual ~ITaskObserver() = default;

    virtual void onTasksUpdated() = 0;
};

#endif // ITASKOBSERVER_H