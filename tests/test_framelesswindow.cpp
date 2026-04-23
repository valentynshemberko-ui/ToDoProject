#include <gtest/gtest.h>
#include <QPixmap>
#include <QApplication>
#include <QTimer>
#include <QTest>
#include <QMouseEvent>
#include <QPushButton>
#include <QGraphicsEffect>
#include <QScreen>
#include <QEnterEvent>
#include <QPropertyAnimation>
#include "../windowEdit/framelessWindow.h"
#include "../windowEdit/snapPreviewWindow.h"

FramelessMessageBox* getOpenMessageBox() {
    for (QWidget* widget : QApplication::topLevelWidgets()) {
        if (auto msgBox = qobject_cast<FramelessMessageBox*>(widget)) {
            if (msgBox->isVisible()) {
                return msgBox;
            }
        }
    }
    return nullptr;
}

class FramelessWindowTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// ==========================================
// Тести для FramelessMessageBox (Статичні методи)
// ==========================================

TEST_F(FramelessWindowTest, MessageBoxInformation) {
    QTimer::singleShot(250, []() {
        auto msgBox = getOpenMessageBox();
        ASSERT_TRUE(msgBox != nullptr);
        
        // Знаходимо кнопку OK і натискаємо її
        auto btns = msgBox->findChildren<QPushButton*>();
        for (auto btn : btns) {
            if (btn->text() == "OK") {
                btn->click();
                return;
            }
        }
        msgBox->reject(); // Фолбек, якщо кнопку не знайдено
    });

    // Виклик заблокує потік, поки таймер не натисне ОК
    FramelessMessageBox::information(nullptr, "Info Test", "This is info.");
    
    // Якщо тест дійшов сюди, значить вікно успішно відкрилось і закрилось
    SUCCEED();
}

TEST_F(FramelessWindowTest, MessageBoxWarning) {
    QTimer::singleShot(250, []() {
        auto msgBox = getOpenMessageBox();
        ASSERT_TRUE(msgBox != nullptr);
        auto btns = msgBox->findChildren<QPushButton*>();
        for (auto btn : btns) if (btn->text() == "OK") btn->click();
    });

    FramelessMessageBox::warning(nullptr, "Warning Test", "This is warning.");
    SUCCEED();
}

TEST_F(FramelessWindowTest, MessageBoxQuestion_Accept) {
    QTimer::singleShot(250, []() {
        auto msgBox = getOpenMessageBox();
        ASSERT_TRUE(msgBox != nullptr);
        auto btns = msgBox->findChildren<QPushButton*>();
        for (auto btn : btns) if (btn->text() == "OK") btn->click();
    });

    bool result = FramelessMessageBox::question(nullptr, "Question Test", "Yes or No?");
    EXPECT_TRUE(result); // OK повертає true
}

TEST_F(FramelessWindowTest, MessageBoxQuestion_Reject) {
    QTimer::singleShot(250, []() {
        auto msgBox = getOpenMessageBox();
        ASSERT_TRUE(msgBox != nullptr);
        auto btns = msgBox->findChildren<QPushButton*>();
        for (auto btn : btns) if (btn->text() == "Cancel") btn->click();
    });

    bool result = FramelessMessageBox::question(nullptr, "Question Test", "Yes or No?");
    EXPECT_FALSE(result); // Cancel повертає false
}

TEST_F(FramelessWindowTest, MessageBoxCritical) {
    QTimer::singleShot(250, []() {
        auto msgBox = getOpenMessageBox();
        ASSERT_TRUE(msgBox != nullptr);
        auto btns = msgBox->findChildren<QPushButton*>();
        for (auto btn : btns) if (btn->text() == "Cancel") btn->click();
    });

    FramelessMessageBox::critical(nullptr, "Critical Test", "This is critical.");
    SUCCEED();
}

// ==========================================
// Тести для FramelessWindow
// ==========================================

TEST_F(FramelessWindowTest, FullscreenToggleOnF4) {
    FramelessWindow w;
    w.resize(400, 400);
    w.show();
    QTest::qWait(100); // Чекаємо відображення вікна

    bool initiallyFullscreen = w.isFullScreen();
    
    // Симулюємо натискання F4
    QTest::keyClick(&w, Qt::Key_F4);
    
    // Оскільки анімація триває 200мс, чекаємо 300мс
    QTest::qWait(300); 
    
    EXPECT_NE(initiallyFullscreen, w.isFullScreen());

    // Повертаємо назад
    QTest::keyClick(&w, Qt::Key_F4);
    QTest::qWait(300);

    EXPECT_EQ(initiallyFullscreen, w.isFullScreen());
}

TEST_F(FramelessWindowTest, DragWindowByTitleBar) {
    FramelessWindow w;
    QWidget* titleBar = new QWidget(&w);
    titleBar->setObjectName("titleBar");
    titleBar->setGeometry(0, 0, 800, 36);
    w.resize(800, 600);
    w.show();
    QTest::qWait(100);

    QPoint startPos = w.pos();
    QPoint pressPos = titleBar->mapToGlobal(QPoint(10, 10));

    // Симулюємо натискання миші на TitleBar
    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPoint(10,10), pressPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&w, &pressEvent);

    // Симулюємо перетягування
    QPoint movePos = pressPos + QPoint(50, 50);
    QMouseEvent moveEvent(QEvent::MouseMove, QPoint(60,60), movePos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&w, &moveEvent);

    // Відпускаємо мишу
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPoint(60,60), movePos, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&w, &releaseEvent);

    // Вікно мало переміститися
    EXPECT_NE(startPos, w.pos());
}

TEST_F(FramelessWindowTest, ResizeWindowFromTopLeft) {
    FramelessWindow w;
    w.resize(400, 400);
    w.show();
    QTest::qWait(100);

    int startWidth = w.width();
    QPoint pressPos = w.mapToGlobal(QPoint(2, 2)); // Координати TopLeft margin (зона ресайзу)

    // Затискаємо мишу в лівому верхньому куті
    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPoint(2,2), pressPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&w, &pressEvent);

    // Тягнемо вліво і вгору
    QPoint movePos = pressPos + QPoint(-50, -50);
    QMouseEvent moveEvent(QEvent::MouseMove, QPoint(-48,-48), movePos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&w, &moveEvent);

    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPoint(-48,-48), movePos, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&w, &releaseEvent);

    // Оскільки ми потягнули лівий край ще лівіше, ширина вікна мала збільшитись
    EXPECT_GT(w.width(), startWidth);
}

TEST_F(FramelessWindowTest, DoubleClickTitleBarMaximizes) {
    FramelessWindow w;
    QWidget* titleBar = new QWidget(&w);
    titleBar->setObjectName("titleBar");
    titleBar->setGeometry(0, 0, 800, 36);
    w.resize(400, 400);
    w.show();
    QTest::qWait(100);

    QPoint clickPos = titleBar->mapToGlobal(QPoint(20, 15));

    // Подвійний клік
    QMouseEvent dblClickEvent(QEvent::MouseButtonDblClick, QPoint(20, 15), clickPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&w, &dblClickEvent);
    
    QTest::qWait(100);
    

    EXPECT_GT(w.width(), 400); 
}

TEST_F(FramelessWindowTest, DialogHasShadowWhenEnabled) {
    FramelessDialog d(nullptr, true); // Shadow enabled
    EXPECT_TRUE(d.graphicsEffect() != nullptr);
}

TEST_F(FramelessWindowTest, DialogResizeFromBottomRight) {
    FramelessDialog d(nullptr, false);
    d.setResizeEnabled(true);
    d.resize(300, 300);
    d.show();
    QTest::qWait(100);

    int startHeight = d.height();
    QPoint pressPos = d.mapToGlobal(QPoint(298, 298)); // BottomRight corner

    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPoint(298,298), pressPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&d, &pressEvent);

    QPoint movePos = pressPos + QPoint(50, 50);
    QMouseEvent moveEvent(QEvent::MouseMove, QPoint(348,348), movePos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&d, &moveEvent);

    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPoint(348,348), movePos, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&d, &releaseEvent);

    EXPECT_GT(d.height(), startHeight);
}



TEST_F(FramelessWindowTest, SnapToScreenEdges) {
    FramelessWindow w;
    QWidget* titleBar = new QWidget(&w);
    titleBar->setObjectName("titleBar");
    titleBar->setGeometry(0, 0, 800, 36);
    w.resize(400, 400);
    w.show();
    QTest::qWait(100);

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect avail = screen->availableGeometry();

    // Затискаємо мишу на TitleBar
    QPoint pressPos = titleBar->mapToGlobal(QPoint(20, 10));
    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPoint(20, 10), pressPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&w, &pressEvent);

    // --- Тест прилипання до ВЕРХНЬОГО краю (Top Snap) ---
    QPoint topEdgePos(avail.x() + 100, avail.y()); // Координата на самому верху
    QMouseEvent moveTop(QEvent::MouseMove, w.mapFromGlobal(topEdgePos), topEdgePos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&w, &moveTop);

    // Відпускаємо мишу - має спрацювати snapPreview->currentType() == Top
    QMouseEvent releaseTop(QEvent::MouseButtonRelease, w.mapFromGlobal(topEdgePos), topEdgePos, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&w, &releaseTop);

    QTest::qWait(50);
    EXPECT_EQ(w.geometry(), avail); // Вікно мало стати на весь доступний екран (isMaximized)

    // --- Тест прилипання до ЛІВОГО краю (Left Snap) ---
    // Знову беремо вікно
    pressPos = titleBar->mapToGlobal(QPoint(20, 10));
    QMouseEvent pressEvent2(QEvent::MouseButtonPress, QPoint(20, 10), pressPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&w, &pressEvent2);

    QPoint leftEdgePos(avail.x(), avail.y() + 100);
    QMouseEvent moveLeft(QEvent::MouseMove, w.mapFromGlobal(leftEdgePos), leftEdgePos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&w, &moveLeft);

    QMouseEvent releaseLeft(QEvent::MouseButtonRelease, w.mapFromGlobal(leftEdgePos), leftEdgePos, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&w, &releaseLeft);

    QTest::qWait(50);
    EXPECT_EQ(w.width(), avail.width() / 2); // Має зайняти рівно ліву половину

    // Аналогічно покривається і Right Snap, але цих двох достатньо для покриття switch.
}


// ==========================================
// 2. Тести оновлення курсора, Enter/Leave та обмежень перетягування у FramelessDialog
// ==========================================
TEST_F(FramelessWindowTest, DialogCursorUpdatesAndEnterLeave) {
    FramelessDialog d(nullptr, false);
    d.setResizeEnabled(true);
    d.resize(300, 300);
    d.show();
    QTest::qWait(100);

    // 1. Тест курсора на краях (updateCursorShape)
    QPoint rightEdge(298, 150);
    QMouseEvent moveEvent(QEvent::MouseMove, rightEdge, d.mapToGlobal(rightEdge), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&d, &moveEvent);
    EXPECT_EQ(d.cursor().shape(), Qt::SizeHorCursor); // Right краю відповідає горизонтальний курсор

    QPoint bottomEdge(150, 298);
    QMouseEvent moveEvent2(QEvent::MouseMove, bottomEdge, d.mapToGlobal(bottomEdge), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&d, &moveEvent2);
    EXPECT_EQ(d.cursor().shape(), Qt::SizeVerCursor); // Bottom краю відповідає вертикальний курсор

    // 2. Тест Enter/Leave Event (має скидати курсор на Arrow)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QEnterEvent enterEvent(QPointF(150, 150), QPointF(150, 150), d.mapToGlobal(QPoint(150, 150)));
#else
    QEvent enterEvent(QEvent::Enter);
#endif
    QApplication::sendEvent(&d, &enterEvent);
    EXPECT_EQ(d.cursor().shape(), Qt::ArrowCursor);

    QEvent leaveEvent(QEvent::Leave);
    QApplication::sendEvent(&d, &leaveEvent);
    EXPECT_EQ(d.cursor().shape(), Qt::ArrowCursor);
}

TEST_F(FramelessWindowTest, DialogDragRespectsScreenBounds) {
    FramelessDialog d(nullptr, false);
    QWidget* titleBar = new QWidget(&d);
    titleBar->setObjectName("titleBar");
    titleBar->setGeometry(0, 0, 300, 36);
    d.resize(300, 300);
    d.show();
    QTest::qWait(100);

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect avail = screen->availableGeometry();

    // Затискаємо TitleBar
    QPoint pressPos = titleBar->mapToGlobal(QPoint(10, 10));
    QMouseEvent press(QEvent::MouseButtonPress, QPoint(10,10), pressPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&d, &press);

    // Тягнемо ВЕРХ за межі екрану (y < avail.y())
    QPoint outOfBoundsTop = QPoint(pressPos.x(), avail.y() - 100);
    QMouseEvent moveUp(QEvent::MouseMove, d.mapFromGlobal(outOfBoundsTop), outOfBoundsTop, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&d, &moveUp);

    // Завдяки перевіркам 'if (newTopLeft.y() < avail.y())', вікно не повинно вийти за верхній край
    EXPECT_GE(d.y(), avail.y());

    // Відпускаємо мишу
    QMouseEvent release(QEvent::MouseButtonRelease, d.mapFromGlobal(outOfBoundsTop), outOfBoundsTop, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&d, &release);
}

TEST_F(FramelessWindowTest, MessageBoxDragging) {
    FramelessMessageBox msgBox("Title", "Message", QMessageBox::Information, nullptr);
    msgBox.show();
    QTest::qWait(100);

    QPoint startPos = msgBox.pos();

    // Знаходимо безпечне місце для кліку (не на кнопках)
    // Координати (10, 10) - це зазвичай фон або заголовок
    QPoint clickLocal(10, 10);
    QPoint clickGlobal = msgBox.mapToGlobal(clickLocal);

    // Натискаємо
    QMouseEvent press(QEvent::MouseButtonPress, clickLocal, clickGlobal, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&msgBox, &press);

    // Тягнемо на 100px вправо і вниз
    QPoint dragGlobal = clickGlobal + QPoint(100, 100);
    QPoint dragLocal = msgBox.mapFromGlobal(dragGlobal);
    QMouseEvent move(QEvent::MouseMove, dragLocal, dragGlobal, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&msgBox, &move);

    // Відпускаємо
    QMouseEvent release(QEvent::MouseButtonRelease, dragLocal, dragGlobal, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&msgBox, &release);

    // Позиція мала змінитись (вікно перетягнули)
    EXPECT_NE(msgBox.pos(), startPos);

    msgBox.close(); // Очищуємо після себе
}

class SnapPreviewTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// ==========================================
// 1. Тест розрахунку геометрії (switch statement) та показу вікна
// ==========================================
TEST_F(SnapPreviewTest, ShowPreviewCalculatesCorrectGeometry) {
    SnapPreviewWindow preview;
    QScreen *screen = QGuiApplication::primaryScreen();
    ASSERT_TRUE(screen != nullptr);

    QRect screenRect = screen->availableGeometry();

    // 1. Тестуємо SnapType::Top (повний екран)
    preview.showPreview(SnapPreviewWindow::SnapType::Top, screen);

    // Перевіряємо, чи виконався код: currentSnap = type; lastRect = target; ... show();
    EXPECT_EQ(preview.currentType(), SnapPreviewWindow::SnapType::Top);
    EXPECT_EQ(preview.geometry(), screenRect);
    EXPECT_TRUE(preview.isVisible());

    // 2. Тестуємо SnapType::Left (ліва половина)
    preview.showPreview(SnapPreviewWindow::SnapType::Left, screen);

    EXPECT_EQ(preview.currentType(), SnapPreviewWindow::SnapType::Left);
    QRect expectedLeft(screenRect.x(), screenRect.y(),
                       screenRect.width() / 2, screenRect.height());
    EXPECT_EQ(preview.geometry(), expectedLeft);

    // 3. Тестуємо SnapType::Right (права половина)
    preview.showPreview(SnapPreviewWindow::SnapType::Right, screen);

    EXPECT_EQ(preview.currentType(), SnapPreviewWindow::SnapType::Right);
    QRect expectedRight(screenRect.x() + screenRect.width() / 2, screenRect.y(),
                        screenRect.width() / 2, screenRect.height());
    EXPECT_EQ(preview.geometry(), expectedRight);
}

// ==========================================
// 2. Тест малювання (paintEvent)
// ==========================================
TEST_F(SnapPreviewTest, PaintEventCoverage) {
    SnapPreviewWindow preview;
    QScreen *screen = QGuiApplication::primaryScreen();

    // Викликаємо вікно, щоб воно отримало розміри
    preview.showPreview(SnapPreviewWindow::SnapType::Left, screen);

    // Створюємо картинку (pixmap) за розмірами нашого прев'ю-вікна
    QPixmap pixmap(preview.size());
    pixmap.fill(Qt::transparent);

    // Метод render() примусово і синхронно викликає paintEvent(QPaintEvent*)
    // Це гарантує 100% покриття малювання (QPainter, setRenderHint, drawRoundedRect)
    preview.render(&pixmap);

    // Перевіряємо, що картинка успішно створена (малювання не впало)
    EXPECT_FALSE(pixmap.isNull());

    // Додатково перевіряємо приховування (має встановити Type::None і сховати вікно)
    preview.hidePreview();
    EXPECT_EQ(preview.currentType(), SnapPreviewWindow::SnapType::None);
    EXPECT_FALSE(preview.isVisible());
}