#include "mainwindow.h"
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("House Web Service");
    resize(1280, 800);
    
    m_webView = new QWebEngineView(this);
    m_webView->load(QUrl("http://localhost:8080"));
    
    setCentralWidget(m_webView);
}

MainWindow::~MainWindow() {
}
