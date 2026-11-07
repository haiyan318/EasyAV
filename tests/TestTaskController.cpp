#include "TestSupport.h"

#include "core/ITaskRunner.h"
#include "model/TaskController.h"

#include <string>
#include <utility>
#include <vector>

using easyav::FileFailure;
using easyav::TaskController;
using easyav::TaskEvent;
using easyav::TaskEventKind;
using easyav::TaskPhase;
using easyav::TaskRequest;
using easyav::TaskItem;

namespace
{
    class FakeRunner final : public easyav::ITaskRunner
    {
    public:
        bool Start(const easyav::TaskRequest& request) override
        {
            startCalled = true;
            lastRequest = request;
            return startOk;
        }

        void Cancel() override { cancelCalled = true; }

        void DrainTaskEvents(std::vector<TaskEvent>& out) override
        {
            out.clear();
            out.swap(pending);
        }

        void EnqueueProbe(const std::vector<std::string>&) override {}
        void DrainProbeResults(std::vector<easyav::ProbeResult>& out) override { out.clear(); }

        const easyav::ToolStatus& Tools() const override { return tools; }
        const std::string& StartErrorText() const override { return startError; }

        void Push(TaskEvent event) { pending.push_back(std::move(event)); }

        bool startOk = true;
        bool startCalled = false;
        bool cancelCalled = false;
        std::string startError;
        easyav::ToolStatus tools;
        TaskRequest lastRequest;
        std::vector<TaskEvent> pending;
    };

    TaskRequest MakeRequest()
    {
        TaskRequest request;
        TaskItem first;
        first.displayName = "a.mp4";
        TaskItem second;
        second.displayName = "b.mp4";
        request.items.push_back(first);
        request.items.push_back(second);
        return request;
    }
}

TEST_CASE(Controller_StartAndProgress)
{
    FakeRunner runner;
    TaskController controller;
    controller.AttachRunner(&runner);

    CHECK(controller.Start(MakeRequest()));
    CHECK(runner.startCalled);
    CHECK(controller.IsRunning());
    CHECK_EQ(controller.FileCount(), 2);

    TaskEvent started;
    started.kind = TaskEventKind::FileStarted;
    started.fileIndex = 0;
    started.fileName = "a.mp4";
    runner.Push(started);

    TaskEvent progress;
    progress.kind = TaskEventKind::FileProgress;
    progress.fileIndex = 0;
    progress.overallProgress = 0.5f;
    progress.fileProgress = 1.0f;
    progress.indeterminate = false;
    runner.Push(progress);

    controller.Tick(2.0f);
    CHECK_NEAR(controller.Progress(), 0.5, 0.001);
    CHECK_STR(controller.CurrentFileName(), "a.mp4");
    CHECK_STR(controller.StatusText(), "正在转换：a.mp4");
    CHECK_NEAR(controller.CurrentFileProgress(), 1.0, 0.001);

    TaskEvent next;
    next.kind = TaskEventKind::FileStarted;
    next.fileIndex = 1;
    next.fileName = "b.mp4";
    runner.Push(next);
    controller.Tick(0.1f);
    CHECK_NEAR(controller.CurrentFileProgress(), 0.0, 0.001);
    CHECK_STR(controller.CurrentFileName(), "b.mp4");
}

TEST_CASE(Controller_CompletionAndFailures)
{
    FakeRunner runner;
    TaskController controller;
    controller.AttachRunner(&runner);
    CHECK(controller.Start(MakeRequest()));

    TaskEvent success;
    success.kind = TaskEventKind::FileSucceeded;
    success.outputPath = "C:/out/a_压缩.mp4";
    success.overallProgress = 0.5f;
    runner.Push(success);

    TaskEvent failure;
    failure.kind = TaskEventKind::FileFailed;
    failure.failure.fileName = "b.mp4";
    failure.failure.userMessage = "这个文件打不开，可能已损坏";
    runner.Push(failure);

    TaskEvent finished;
    finished.kind = TaskEventKind::TaskFinished;
    finished.fileCount = 2;
    runner.Push(finished);

    controller.Tick(1.0f);

    CHECK(controller.HasFinished());
    CHECK_EQ(controller.SucceededCount(), 1);
    CHECK_EQ(controller.FailedCount(), 1);
    CHECK_NEAR(controller.Progress(), 1.0, 0.001);
    CHECK_STR(controller.LastOutputPath(), "C:/out/a_压缩.mp4");
    CHECK_STR(controller.StatusText(), "完成 1 个，1 个失败");

    CHECK(controller.ConsumeCompleted());
    CHECK(!controller.ConsumeCompleted());

    const std::vector<FileFailure> failures = controller.ConsumeFailures();
    CHECK_EQ(failures.size(), static_cast<std::size_t>(1));
    CHECK_STR(failures.front().userMessage, "这个文件打不开，可能已损坏");
    CHECK_EQ(controller.ConsumeFailures().size(), static_cast<std::size_t>(0));
}

TEST_CASE(Controller_CancelWaitsForEvent)
{
    FakeRunner runner;
    TaskController controller;
    controller.AttachRunner(&runner);
    CHECK(controller.Start(MakeRequest()));

    controller.Cancel();
    CHECK(runner.cancelCalled);
    CHECK(controller.IsRunning());
    CHECK_STR(controller.StatusText(), "正在取消…");

    TaskEvent canceled;
    canceled.kind = TaskEventKind::TaskCanceled;
    runner.Push(canceled);
    controller.Tick(0.5f);

    CHECK(controller.Phase() == TaskPhase::Canceled);
    CHECK_STR(controller.StatusText(), "已取消");
}

TEST_CASE(Controller_StartFailureReportsRunnerError)
{
    FakeRunner runner;
    runner.startOk = false;
    runner.startError = "找不到处理组件";

    TaskController controller;
    controller.AttachRunner(&runner);

    CHECK(!controller.Start(MakeRequest()));
    CHECK(controller.Phase() == TaskPhase::Idle);
    CHECK_STR(controller.StartErrorText(), "找不到处理组件");
}

TEST_CASE(Controller_IndeterminateProgressHasNoEta)
{
    FakeRunner runner;
    TaskController controller;
    controller.AttachRunner(&runner);
    CHECK(controller.Start(MakeRequest()));

    TaskEvent progress;
    progress.kind = TaskEventKind::FileProgress;
    progress.overallProgress = 0.3f;
    progress.indeterminate = true;
    runner.Push(progress);
    controller.Tick(1.0f);

    CHECK(controller.Indeterminate());
    CHECK_NEAR(controller.EtaSeconds(), -1.0, 0.001);
}

TEST_CASE(Controller_NoRunnerUsesEmptyTools)
{
    TaskController controller;
    CHECK(!controller.Start(MakeRequest()));
    CHECK(!controller.Tools().ffmpegFound);
    CHECK(!controller.StartErrorText().empty());
}
