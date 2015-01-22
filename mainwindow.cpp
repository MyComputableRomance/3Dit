#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "stlexporter.h"

//Static:
MessageDialog* MainWindow::msgDlg=NULL;
void MainWindow::showMessageBox(QString title, QString content) {
    delete msgDlg;
    msgDlg = new MessageDialog(title, content, NULL);
    msgDlg->show();
}

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    gt = new GThread(false, 0);
    pgt = new GThread(true, 0);

    //Connect:
    connect(ui->title,SIGNAL(titleDoubleClicked()),this,SLOT(on_max_clicked()));
    connect(ui->title,SIGNAL(titleMoved(QPoint)),this,SLOT(onTitleMoved(QPoint)));

    //Do some initialization:
    QStackedLayout* layout = (QStackedLayout*)ui->up->layout();
    layout->setStackingMode(QStackedLayout::StackAll);
    ui->tabWidget->setCurrentIndex(0);
    ui->up->setCurrentIndex(1); //"Start" page.
    ui->left->setCurrentIndex(0); //"Expanded" page.
    ui->left->setFixedWidth(150);

    //Frameless window:
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    this->setMouseTracking(true);
    //Move to center:
    QDesktopWidget* desktopWidget =QApplication::desktop();
    QRect screenRect =desktopWidget->availableGeometry();
    this->move(screenRect.width()/2 - width()/2, screenRect.height()/2 - height()/2);

    //Maxnormal:
    maxNormal = false;
    //Mouse pressed?
    pressed = false;
    //Which direction to expand?
    left = right = bottom = top = false;

    //Dialogs:
    scaleDlg = new ScaleDialog();
    transDlg = new TranslateDialog();
    rotateDlg = new RotateDialog();
    alignDlg = new AlignDialog();
    createModelDlg = new CreateModelDialog();
    //File Name to Save:
    fileName = "";
    //Viewport:
    viewport = new Viewport(NULL, this);
    QHBoxLayout* hboxLayout = (QHBoxLayout*)ui->viewportWidget->layout();
    hboxLayout->addWidget(viewport);
    //FileCmb:
    ui->fileCmb->setEnabled(false);
    ui->fileCmb->addItem("");

    //Connect:
    connect(this,SIGNAL(coordinateChanged()),this,SLOT(resize()));
    connect(viewport,SIGNAL(selectionChanged()),this,SLOT(updateCmb()));
    connect(viewport,SIGNAL(selectionChanged_for_AlignDlg(int)),alignDlg,SLOT(updateDialog(int)));

    //Connect the settings of the parameter:
    connect(ui->resolution,SIGNAL(currentIndexChanged(int)),viewport,SLOT(change()));
    connect(ui->fillDensity,SIGNAL(currentIndexChanged(int)),viewport,SLOT(change()));
    connect(ui->perimeter,SIGNAL(currentIndexChanged(int)),viewport,SLOT(change()));

    //Connect operations from the buttons:
    connect(ui->left_import_c_1,SIGNAL(clicked()),this,SLOT(doCreate()));
    connect(ui->left_import_e_2,SIGNAL(clicked()),this,SLOT(doCreate()));
    connect(ui->left_export_c_1,SIGNAL(clicked()),this,SLOT(doExport()));
    connect(ui->left_export_e_2,SIGNAL(clicked()),this,SLOT(doExport()));
    connect(ui->left_preview_c_1,SIGNAL(toggled(bool)),this,SLOT(doPreview(bool)));
    connect(ui->left_preview_e_2,SIGNAL(toggled(bool)),this,SLOT(doPreview(bool)));
    connect(ui->newModel,SIGNAL(clicked()),this,SLOT(doCreate()));
    connect(ui->open,SIGNAL(clicked()),this,SLOT(doImport()));
    connect(ui->save,SIGNAL(clicked()),this,SLOT(doSave()));
    connect(ui->saveAs,SIGNAL(clicked()),this,SLOT(doExport()));
    connect(ui->doCopy,SIGNAL(clicked()),this,SLOT(doCopy()));
    connect(ui->doPaste,SIGNAL(clicked()),this,SLOT(doPaste()));
    connect(ui->doDelete,SIGNAL(clicked()),this,SLOT(doDelete()));
    connect(ui->doSelectAll,SIGNAL(clicked()),this,SLOT(doSelectAll()));
    connect(ui->doRedo,SIGNAL(clicked()),this,SLOT(doRedo()));
    connect(ui->doUndo,SIGNAL(clicked()),this,SLOT(doUndo()));

    //Connect the operations from dialogs:
    connect(scaleDlg,SIGNAL(onScale(double,double,double)),this,SLOT(doScale(double,double,double)));
    connect(transDlg,SIGNAL(onTranslate(double,double,double)),this,SLOT(doPosition(double,double,double)));
    connect(rotateDlg,SIGNAL(onRotate(double,double,double)),this,SLOT(doRotation(double,double,double)));
    connect(alignDlg,SIGNAL(onAlign(QString)),this,SLOT(doAlign(QString)));
    connect(alignDlg,SIGNAL(onStick(bool)),this,SLOT(doStick(bool)));
    connect(createModelDlg,SIGNAL(createPrefab(int)),this,SLOT(doImportPrefab(int)));
    connect(viewport,SIGNAL(onScale(double,double,double)),this,SLOT(doScaleDrag(double,double,double)));
    connect(viewport,SIGNAL(onMove(double,double,double)),this,SLOT(doPositionDrag(double,double,double)));
    connect(viewport,SIGNAL(onRotate(double,double,double)),this,SLOT(doRotationDrag(double,double,double)));

    // draw preview
    connect(pgt, SIGNAL(drawPreview()),this,SLOT(onDrawPreview()));
}

MainWindow::~MainWindow()
{
    delete scaleDlg;
    delete transDlg;
    delete rotateDlg;
    delete alignDlg;
    if(msgDlg)
        delete msgDlg;

    delete ui;

    delete gt;
    delete pgt;
}

QRect MainWindow::getRect(){
    QRect rec = rect();
    rec.setLeft(rec.left() - 1);
    rec.setTop(rec.top() - 1);
    return rec;
}

void MainWindow::paintEvent(QPaintEvent *)
{
    if(!maxNormal){
        //Draw background:
        QPainter background(this);
        background.setBrush(QColor(66,66,66));
        background.drawRect(getRect());

        //Draw mask:
        QBitmap objBitmap(size());
        QPainter painter(&objBitmap);
        painter.fillRect(rect(),Qt::white);
        painter.setBrush(QColor(0,0,0));
        painter.drawRoundedRect(rect(),10,10);
        this->setMask(objBitmap);
    }else{
        //Draw background:
        QPainter background(this);
        background.setBrush(QColor(66,66,66));
        background.drawRect(getRect());

        //Clear mask:
        this->clearMask();
    }
}

void MainWindow::on_left_expand_btn_clicked()
{
    ui->left->setCurrentIndex(0); //"Expanded" page.
    ui->left->setFixedWidth(150);
}

void MainWindow::on_left_collapse_btn_clicked()
{
    ui->left->setCurrentIndex(1); //"Collapsed" page.
    ui->left->setFixedWidth(60);
}

void MainWindow::on_close_clicked()
{
    scaleDlg->close();
    alignDlg->close();
    rotateDlg->close();
    transDlg->close();
    close();
}

void MainWindow::on_min_clicked()
{
    showMinimized();
}

void MainWindow::doMaximize(){
    showMaximized();
    ui->max->setStyleSheet("border-image: url(:/Tabwidget/Image/Tabwidget/Reset.png);");
    //Set layout:
    QHBoxLayout* layout = (QHBoxLayout*)ui->centralWidget->layout();
    layout->setMargin(0);
    ui->centralWidget->setLayout(layout);
}

void MainWindow::doNormalize(){
    showNormal();
    ui->max->setStyleSheet("border-image: url(:/Tabwidget/Image/Tabwidget/Max.png);");
    //Set layout:
    QHBoxLayout* layout = (QHBoxLayout*)ui->centralWidget->layout();
    layout->setMargin(5);
    ui->centralWidget->setLayout(layout);
}

void MainWindow::on_max_clicked()
{
    if(maxNormal)
        doNormalize();
    else
        doMaximize();
    maxNormal = !maxNormal;
}

void MainWindow::onTitleMoved(QPoint p){
    if(maxNormal)
        return;
    this->move(this->pos()+p);
}

void MainWindow::mousePressEvent(QMouseEvent *me){
    if(me->button() == Qt::LeftButton){
        pressed = true;
        start_point = me->globalPos();
        start_geometry = geometry();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *me){
    if(maxNormal)
        return;
    if(pressed){
        QPoint delta = me->globalPos() - start_point;
        int dx = delta.x();
        int dy = delta.y();

        QRect g = start_geometry;
        int minW = minimumWidth();
        int minH = minimumHeight();
        if (left){
            g.setLeft(g.left() + dx);
            if(g.width() < minW)
                g.setLeft(g.right() - minW);
        }
        if (right){
            g.setRight(g.right() + dx);
            if(g.width() < minW)
                g.setRight(g.left() + minW);
        }
        if (top){
            g.setTop(g.top() + dy);
            if(g.height() < minH)
                g.setTop(g.bottom() - minH);
        }
        if (bottom){
            g.setBottom(g.bottom() + dy);
            if(g.height() < minH)
                g.setBottom(g.top() + minH);
        }
        setGeometry(g);
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *){
    pressed = false;
    setCursor(Qt::ArrowCursor);
}

void MainWindow::resizeEvent(QResizeEvent *re){
    // Remeber to set the size of centralWidget 1 pixel smaller than mainWindow,
    // or some unexpected error will occur!
    ui->centralWidget->setGeometry(0,0,re->size().width()-1,re->size().height()-1);
}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, long *)
{
    Q_UNUSED(eventType);
    MSG* msg = reinterpret_cast<MSG*>(message);

    switch(msg->message){
    case WM_SETCURSOR:{
        if(maxNormal)
            return false;

        int x = msg->pt.x - this->frameGeometry().x();
        int y = msg->pt.y - this->frameGeometry().y();
        QRect r = getRect();
        left = qAbs(x - r.left()) < 5;
        right = qAbs(x - r.right()) < 5;
        bottom = qAbs(y - r.bottom()) < 5;
        top = qAbs(y - r.top()) < 5;

        if(left&&bottom || right&&top)
            setCursor(Qt::SizeBDiagCursor);
        else if(left&&top || right&&bottom)
            setCursor(Qt::SizeFDiagCursor);
        else if(top || bottom)
            setCursor(Qt::SizeVerCursor);
        else if(left || right)
            setCursor(Qt::SizeHorCursor);
        else
            setCursor(Qt::ArrowCursor);
    }
    }
    return false;
}

void MainWindow::on_checkBox_toggled(bool checked)
{
    viewport->drawMesh(checked);
}

void MainWindow::refreshCmb(){
    while(ui->fileCmb->count() > 1)
        ui->fileCmb->removeItem(1); //The first item remains.
    QMultiMap<QString, MeshInfo*>::iterator it;
    for(it=viewport->meshes->begin();it!=viewport->meshes->end();it++)
        ui->fileCmb->addItem(it.value()->getFullName());
    //If there're no file item:
    if(ui->fileCmb->count() == 1)
        ui->fileCmb->setEnabled(false);
    else
        ui->fileCmb->setEnabled(true);
}

void MainWindow::updateCmb(){
    /* Attention:
     * The pos of the operator is [geometry center];
     * The center shown in transDlg is [box center].
     */
    QComboBox* fileCmb = ui->fileCmb;
    int size = viewport->getSelection()->size();
    if(size == 0){
        fileCmb->setCurrentIndex(0);   
        ui->model_scale->setText("(No Model Selected)");
        scaleDlg->setValue(0, 0, 0, 1);
        transDlg->setValue(0, 0, 0);
        rotateDlg->setValue(0, 0, 0);
    }else if(size >= 2){
        fileCmb->setCurrentIndex(0);
        aiVector3D selection_max = viewport->selection_max;
        aiVector3D selection_min = viewport->selection_min;
        aiVector3D boxSize = selection_max - selection_min;
        QString str;
        str.sprintf("Model Size: %.2f*%.2f*%.2f(mm*mm*mm)",boxSize.x, boxSize.y, boxSize.z);
        ui->model_scale->setText(str);

        aiVector3D global_box_center = viewport->global_box_center;
        scaleDlg->setValue(boxSize.x, boxSize.y, boxSize.z, 1);
        transDlg->setValue(global_box_center.x, global_box_center.y, global_box_center.z);
        rotateDlg->setValue(0, 0, 0);
    }else{
        MeshInfo* info = viewport->getSelection()->at(0)->mesh;
        for(int i=0;i<fileCmb->count();i++)
            if(fileCmb->itemText(i) == info->getFullName())
                fileCmb->setCurrentIndex(i);
        ui->model_scale->setText(info->getSizeText());

        scaleDlg->setValue(info->getLength(), info->getWidth(), info->getHeight(), 1);
        transDlg->setValue(info->getBoxCenter().x, info->getBoxCenter().y, info->getBoxCenter().z);
        rotateDlg->setValue(0, 0, 0);
    }
}

/* The difference between currentTextChanged and activated is that
 * currentTextChanged: emitted when the user selects an item and when setCurrentIndex(int i) is executed;
 * activated: emitted only when the user selects an item.
 */
void MainWindow::on_fileCmb_activated(QString fullName)
{
    //If mode == OPT_ALIGN, remember to set mode == NULL!
    on_left_alignment_c_1_clicked(false);
    viewport->updateMeshes(fullName);
    updateSizeLbl(fullName);
}

void MainWindow::updateSizeLbl(QString fullName){
    MeshInfo* selected = viewport->getModel(fullName);
    if(selected){
        ui->model_scale->setText(selected->getSizeText());

        scaleDlg->setValue(selected->getLength(), selected->getWidth(), selected->getHeight(), 1);
        transDlg->setValue(selected->getBoxCenter().x, selected->getBoxCenter().y, selected->getBoxCenter().z);
    }else{
        ui->model_scale->setText("(No Model Selected)");

        scaleDlg->setValue(0, 0, 0, 1);
        transDlg->setValue(0, 0, 0);
    }
    rotateDlg->setValue(0, 0, 0);
}

//File I/O:
void MainWindow::doImport(){
    //If mode == OPT_ALIGN, remember to set mode == NULL!
    on_left_alignment_c_1_clicked(false);

    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Import 3D Model", ".", "3D Model Files (*.obj *.stl *.3ds *.ply)");
    if(fileNames.length() == 0)
        return;

    int failedCount = 0;
    for(int i=0;i<fileNames.length();i++){
        QString fileName = fileNames.at(i);
        bool successful = viewport->loadFile(fileName);
        if(!successful)
            failedCount++;
    }

    if(failedCount != 0)
        MainWindow::showMessageBox("File Import", QString("There are %1 file(s) failed to import.").arg(failedCount));
    //Change:
    if(fileNames.length() != failedCount)
        viewport->setChanged(true);
    viewport->isRecorded = false;
}

void MainWindow::doExport(){
    int size = viewport->meshNum();
    if(size == 0){
        MainWindow::showMessageBox("File Export", "There is no mesh in the viewport.");
        return;
    }
    QString file = QFileDialog::getSaveFileName(this, "Export 3D Model", ".", "STL Model Files (*.stl)");
    if(file.length() == 0)
        return;
    //Start exporting process:
    vector<const aiScene *> scs;
    for(int i=0;i<viewport->meshNum();i++)
        scs.push_back(viewport->getModelAt(i)->getMesh()->scene);
    bool successful = AsciiStlExporter(file.toStdString(), scs);
    if(!successful)
        MainWindow::showMessageBox("File Export", "Exporting is failed due to some errors.");
    return;
}

void MainWindow::doSave(){
    int size = viewport->meshNum();
    if(size == 0){
        MainWindow::showMessageBox("File Saving", "There is no mesh in the viewport.");
        return;
    }
    QString destName;
    if(fileName == ""){
        //If the user haven't chosen the file to save:
        destName = QFileDialog::getSaveFileName(this, "Save 3D Model", ".", "STL Model Files (*.stl)");
        if(destName.length() == 0)
            return;
        else
            fileName = destName;
    }else
        destName = fileName;
    //Start saving process:
    vector<const aiScene *> scs;
    for(int i=0;i<viewport->meshNum();i++)
        scs.push_back(viewport->getModelAt(i)->getMesh()->scene);
    bool successful = AsciiStlExporter(destName.toStdString(), scs);
    if(!successful)
        MainWindow::showMessageBox("File Saving", "Saving is failed due to some errors.");
    return;
}

//Operations:
void MainWindow::resize(){
    QVector<SelectionInfo*>* selection = viewport->getSelection();
    if(selection->size() == 1){
         MeshInfo* info = selection->at(0)->mesh;
         ui->model_scale->setText(info->getSizeText());
         scaleDlg->setValue(info->getLength(), info->getWidth(), info->getHeight(), 1);
         transDlg->setValue(info->getBoxCenter().x, info->getBoxCenter().y, info->getBoxCenter().z);
         rotateDlg->setValue(0, 0, 0);
    }else{
        aiVector3D global_box_center = viewport->global_box_center;
        aiVector3D selection_max = viewport->selection_max;
        aiVector3D selection_min = viewport->selection_min;
        aiVector3D boxSize = selection_max - selection_min;

        QString str;
        str.sprintf("Model Size: %.2f*%.2f*%.2f(mm*mm*mm)",boxSize.x, boxSize.y, boxSize.z);
        ui->model_scale->setText(str);
        scaleDlg->setValue(boxSize.x, boxSize.y, boxSize.z, 1);
        transDlg->setValue(global_box_center.x, global_box_center.y, global_box_center.z);
        rotateDlg->setValue(0, 0, 0);
    }
}

void MainWindow::doScale(double nx, double ny, double nz){
    int size = viewport->meshNum();
    if(size == 0){
        MainWindow::showMessageBox("Scaling", "There is no mesh in the viewport.");
        return;
    }
    int selectedNum = viewport->getSelection()->size();
    if(selectedNum == 0){
        MainWindow::showMessageBox("Scaling", "No model is selected.");
        return;
    }

    //Do scale:
    //If multiselection, the center of the scaling is [the geometry center of the first selected mesh].
    QVector<SelectionInfo*>* selection = viewport->getSelection();
    aiVector3D ctr = selection->at(0)->mesh->getMesh()->geometry_center;
    aiVector3D boxSize = viewport->selection_max - viewport->selection_min;
    double tx = nx / boxSize.x;
    double ty = ny / boxSize.y;
    double tz = nz / boxSize.z;

    tx = ROUNDTOONE(tx);
    ty = ROUNDTOONE(ty);
    tz = ROUNDTOONE(tz);
    if(tx==1 && ty==1 && tz==1)
        return;

    //Record the world status for UNDO:
    viewport->record();
    for (int i=0;i<selectedNum;i++){
        MeshInfo* info = selection->at(i)->mesh;
        info->doScale(tx, ty, tz, ctr);
    }
    viewport->updateSelectionBoundingBox();
    viewport->boundingBoxAdjustment();
    viewport->setChanged(true);
    emit coordinateChanged();
}

void MainWindow::doPosition(double px, double py, double pz){
    int size = viewport->meshNum();
    if(size == 0){
        MainWindow::showMessageBox("Moving", "There is no mesh in the viewport.");
        return;
    }
    int selectedNum = viewport->getSelection()->size();
    if(selectedNum == 0){
        MainWindow::showMessageBox("Scaling", "No model is selected.");
        return;
    }

    //Do position:
    QVector<SelectionInfo*>* selection = viewport->getSelection();
    aiVector3D ctr = viewport->global_box_center;
    double dx = px - ctr.x;
    double dy = py - ctr.y;
    double dz = pz - ctr.z;

    dx = ROUNDTOZERO(dx);
    dy = ROUNDTOZERO(dy);
    dz = ROUNDTOZERO(dz);
    if(dx==0 && dy==0 && dz==0)
        return;

    //Record the world status for UNDO:
    viewport->record();
    for (int i=0;i<selectedNum;i++){
        MeshInfo* info = selection->at(i)->mesh;
        info->doPosition(dx, dy, dz);
    }
    viewport->updateSelectionBoundingBox();
    viewport->boundingBoxAdjustment();
    viewport->setChanged(true);
    emit coordinateChanged();
}

void MainWindow::doRotation(double rx, double ry, double rz)
{
    int size = viewport->meshNum();
    if(size == 0){
        MainWindow::showMessageBox("Rotation", "There is no mesh in the viewport.");
        return;
    }
    int selectedNum = viewport->getSelection()->size();
    if(selectedNum == 0){
        MainWindow::showMessageBox("Scaling", "No model is selected.");
        return;
    }

    //Do rotation:
    //If multiselection, the center of the rotation is [the geometry center of the first selected mesh].
    QVector<SelectionInfo*>* selection = viewport->getSelection();
    aiVector3D ctr = selection->at(0)->mesh->getMesh()->geometry_center;

    rx = ROUNDTOZERO(rx);
    ry = ROUNDTOZERO(ry);
    rz = ROUNDTOZERO(rz);
    if(rx==0 && ry==0 && rz==0)
        return;

    //Record the world status for UNDO:
    viewport->record();
    for (int i=0;i<selectedNum;i++){
        MeshInfo* info = selection->at(i)->mesh;
        info->doRotation(rx, ry, rz, ctr);
    }
    viewport->updateSelectionBoundingBox();
    viewport->boundingBoxAdjustment();
    viewport->setChanged(true);
    emit coordinateChanged();
}

void MainWindow::doAlign(QString selectedText){
    MeshInfo* info = viewport->getSelection()->at(0)->mesh;
    aiVector3D normal;
    if(selectedText == "Negative Z")
        normal.Set(0,0,1);
    else if(selectedText == "Positive Z")
        normal.Set(0,0,-1);
    else if(selectedText == "Negative Y")
        normal.Set(0,1,0);
    else if(selectedText == "Positive Y")
        normal.Set(0,-1,0);
    else if(selectedText == "Negative X")
        normal.Set(1,0,0);
    else if(selectedText == "Positive X")
        normal.Set(-1,0,0);
    else if(selectedText == "Custom Face")
        normal = viewport->getSelection()->at(1)->mesh->getSelectedNormal();
    else
        return;

    //Record the world status for UNDO:
    viewport->record();

    info->doAlign(normal);
    info->boundingBoxAdjustment(); //Don't adjust the destination.
    viewport->updateSelectionBoundingBox();
    viewport->setChanged(true);
    emit coordinateChanged();
}

void MainWindow::doStick(bool flag){
    MeshInfo* info = viewport->getSelection()->at(0)->mesh;

    //Record the world status for UNDO:
    viewport->record();

    if(flag){
        aiVector3D normal, center;
        normal = viewport->getSelection()->at(1)->mesh->getSelectedNormal();
        center = viewport->getSelection()->at(1)->mesh->getSelectedCenter();
        info->doStick(normal, center);
    }else
        info->doStickToBottom();

    info->boundingBoxAdjustment();
    viewport->updateSelectionBoundingBox();
    viewport->setChanged(true);
    emit coordinateChanged();
}

void MainWindow::doScaleDrag(double tx, double ty, double tz)
{
    //Do not need to check the mesh num and the selected num.
    //Do not need to do bounding box adjustment during the transform.
    //Just do it when the mouse is released (implemented in viewport.cpp).
    QVector<SelectionInfo*>* selection = viewport->getSelection();
    int selectedNum = selection->size();
    aiVector3D ctr = selection->at(0)->mesh->getMesh()->geometry_center;

    tx = ROUNDTOONE(tx);
    ty = ROUNDTOONE(ty);
    tz = ROUNDTOONE(tz);
    if(tx==1 && ty==1 && tz==1)
        return;

    for (int i=0;i<selectedNum;i++){
        MeshInfo* info = selection->at(i)->mesh;
        info->doScale(tx, ty, tz, ctr);
    }
    viewport->updateSelectionBoundingBox();
    viewport->setChanged(true);
    emit coordinateChanged();
}

void MainWindow::doPositionDrag(double dx, double dy, double dz)
{
    //Do not need to check the mesh num and the selected num.
    //Do not need to do bounding box adjustment during the transform.
    //Just do it when the mouse is released (implemented in viewport.cpp).
    QVector<SelectionInfo*>* selection = viewport->getSelection();
    int selectedNum = selection->size();

    dx = ROUNDTOZERO(dx);
    dy = ROUNDTOZERO(dy);
    dz = ROUNDTOZERO(dz);
    if(dx==0 && dy==0 && dz==0)
        return;

    for (int i=0;i<selectedNum;i++){
        MeshInfo* info = selection->at(i)->mesh;
        info->doPosition(dx, dy, dz);
    }
    viewport->updateSelectionBoundingBox();
    viewport->setChanged(true);
    emit coordinateChanged();
}

void MainWindow::doRotationDrag(double rx, double ry, double rz)
{
    //Do not need to check the mesh num and the selected num.
    //Do not need to do bounding box adjustment during the transform.
    //Just do it when the mouse is released (implemented in viewport.cpp).
    QVector<SelectionInfo*>* selection = viewport->getSelection();
    int selectedNum = selection->size();
    aiVector3D ctr = selection->at(0)->mesh->getMesh()->geometry_center;

    rx = ROUNDTOZERO(rx);
    ry = ROUNDTOZERO(ry);
    rz = ROUNDTOZERO(rz);
    if(rx==0 && ry==0 && rz==0)
        return;

    for (int i=0;i<selectedNum;i++){
        MeshInfo* info = selection->at(i)->mesh;
        info->doRotation(rx, ry, rz, ctr);
    }
    viewport->updateSelectionBoundingBox();
    viewport->setChanged(true);
    emit coordinateChanged();
}

/************ Below are functions controlling scaling, translating and rotating buttons. ************/
//Cancel the previous checked status.
//And reset the new status.
void MainWindow::setChecked(int o, int n)
{
    switch(o){
    case OPT_SCALE:{
        ui->left_scale_c_1->setChecked(false);
        ui->left_scale_e_2->setChecked(false);
        scaleDlg->hide();
        break;
    }
    case OPT_TRANS:{
        ui->left_move_c_1->setChecked(false);
        ui->left_move_e_2->setChecked(false);
        transDlg->hide();
        break;
    }
    case OPT_ROTATE:{
        ui->left_rotate_c_1->setChecked(false);
        ui->left_rotate_e_2->setChecked(false);
        rotateDlg->hide();
        break;
    }
    case OPT_ALIGN:{
        ui->left_alignment_c_1->setChecked(false);
        ui->left_alignment_e_2->setChecked(false);
        alignDlg->hide();
        break;
    }
    default:
        break;
    }

    switch(n){
    case OPT_SCALE:{
        ui->left_scale_c_1->setChecked(true);
        ui->left_scale_e_2->setChecked(true);
        scaleDlg->showFront();
        break;
    }
    case OPT_TRANS:{
        ui->left_move_c_1->setChecked(true);
        ui->left_move_e_2->setChecked(true);
        transDlg->showFront();
        break;
    }
    case OPT_ROTATE:{
        ui->left_rotate_c_1->setChecked(true);
        ui->left_rotate_e_2->setChecked(true);
        rotateDlg->showFront();
        break;
    }
    case OPT_ALIGN:{
        ui->left_alignment_c_1->setChecked(true);
        ui->left_alignment_e_2->setChecked(true);
        alignDlg->showFront();
        break;
    }
    default:
        break;
    }

    //If set the mode to [ALIGN],
    //Need to clear all selection in viewport:
    if(n == OPT_ALIGN){
        viewport->clearSelection();
        viewport->updateSelection();
        viewport->updateSelectionBoundingBox();
        emit viewport->selectionChanged();
    }

    //If set the mode from [ALIGN],
    //Need to change the mesh face selection to full mesh selection:
    //Do not need to updateSelectionBoundingBox and emit selectionChanged.
    if(o == OPT_ALIGN){
        QVector<SelectionInfo*>* selection = viewport->getSelection();
        for(int i=0;i<selection->size();i++){
            SelectionInfo* info = selection->at(i);
            info->sceneMeshIndex = -999;
            info->meshFaceIndex = -1;
        }
        viewport->updateSelection();
    }
}

void MainWindow::on_left_scale_c_1_clicked(bool checked)
{
    if (checked){
        setChecked(viewport->operation_checked, OPT_SCALE);
        viewport->operation_checked = OPT_SCALE;
    }
    else{
        setChecked(viewport->operation_checked, OPT_NONE);
        viewport->operation_checked = OPT_NONE;
    }
}

void MainWindow::on_left_scale_e_2_clicked(bool checked)
{
    on_left_scale_c_1_clicked(checked);
}

void MainWindow::on_left_rotate_c_1_clicked(bool checked)
{
    if (checked){
        setChecked(viewport->operation_checked, OPT_ROTATE);
        viewport->operation_checked = OPT_ROTATE;
    }
    else{
        setChecked(viewport->operation_checked, OPT_NONE);
        viewport->operation_checked = OPT_NONE;
    }
}

void MainWindow::on_left_rotate_e_2_clicked(bool checked)
{
    on_left_rotate_c_1_clicked(checked);
}

void MainWindow::on_left_move_c_1_clicked(bool checked)
{
    if (checked){
        setChecked(viewport->operation_checked, OPT_TRANS);
        viewport->operation_checked = OPT_TRANS;
    }
    else{
        setChecked(viewport->operation_checked, OPT_NONE);
        viewport->operation_checked = OPT_NONE;
    }
}

void MainWindow::on_left_move_e_2_clicked(bool checked)
{
    on_left_move_c_1_clicked(checked);
}

void MainWindow::on_left_alignment_c_1_clicked(bool checked)
{
    if (checked){
        setChecked(viewport->operation_checked, OPT_ALIGN);
        viewport->operation_checked = OPT_ALIGN;
    }
    else{
        setChecked(viewport->operation_checked, OPT_NONE);
        viewport->operation_checked = OPT_NONE;
    }
}

void MainWindow::on_left_alignment_e_2_clicked(bool checked)
{
    on_left_alignment_c_1_clicked(checked);
}

/************ Below are functions controlling creating models. ************/
void MainWindow::doCreate()
{
    createModelDlg->showFront();
}

void MainWindow::doImportModel(QString filepath)
{
    bool isSuccessful = viewport->loadFile(filepath);
    if(!isSuccessful)
        MainWindow::showMessageBox("Create",QString("Creating model is failed."));
    else
        viewport->setChanged(true);
}

void MainWindow::doImportPrefab(int index){
    QString filePath;
    switch(index){
    case 1:
        filePath = "Models/cone.stl";
        break;
    case 2:
        filePath = "Models/cylinder.stl";
        break;
    case 3:
        filePath = "Models/ring.stl";
        break;
    case 4:
        filePath = "Models/triangular_pyramid.stl";
        break;
    case 5:
        filePath = "Models/triangular_prism.stl";
        break;
    case 6:
        filePath = "Models/triangular_ring.stl";
        break;
    case 7:
        filePath = "Models/rectangular_pyramid.stl";
        break;
    case 8:
        filePath = "Models/rectangular_prism.stl";
        break;
    case 9:
        filePath = "Models/rectangular_ring.stl";
        break;
    case 10:
        filePath = "Models/pentagonal_pyramid.stl";
        break;
    case 11:
        filePath = "Models/pentagonal_prism.stl";
        break;
    case 12:
        filePath = "Models/pentagonal_ring.stl";
        break;
    case 13:
        filePath = "Models/hexagonal_pyramid.stl";
        break;
    case 14:
        filePath = "Models/hexagonal_prism.stl";
        break;
    case 15:
        filePath = "Models/hexagonal_ring.stl";
        break;
    case 16:
        filePath = "Models/seven_pyramid.stl";
        break;
    case 17:
        filePath = "Models/seven_prism.stl";
        break;
    case 18:
        filePath = "Models/seven_ring.stl";
        break;
    case 19:
        filePath = "Models/eight_pyramid.stl";
        break;
    case 20:
        filePath = "Models/eight_prism.stl";
        break;
    case 21:
        filePath = "Models/eight_ring.stl";
        break;
    case 22:
        filePath = "Models/sphere.stl";
        break;
    default:
        return;
    }
    doImportModel(filePath);
}

/************ Below are functions controlling copy and paste etc. ************/
void MainWindow::doCopy(){
    viewport->meshCopy();
}

void MainWindow::doPaste(){
    //If mode == OPT_ALIGN, remember to set mode == NULL!
    on_left_alignment_c_1_clicked(false);
    viewport->meshPaste();
}

void MainWindow::doSelectAll(){
    viewport->selectAll();
}

void MainWindow::doDelete(){
    viewport->doDelete();
}

void MainWindow::doRedo(){
    viewport->redo();
}

void MainWindow::doUndo(){
    viewport->undo();
}

//Get parameters from UI:
void MainWindow::getParameter(int& perimeter, double& density, double& resolution){
    int peri_index = ui->perimeter->currentIndex();
    int den_index = ui->fillDensity->currentIndex();
    perimeter = peri_index + 1;
    density = den_index * 0.2;

    int res_index = ui->resolution->currentIndex();
    switch(res_index){
    case 0:
        resolution = 0.05;
        break;
    case 1:
        resolution = 0.08;
        break;
    case 2:
        resolution = 0.1;
        break;
    case 3:
        resolution = 0.2;
    }
}

//Print:
void MainWindow::on_left_print_c_1_clicked()
{
    int size = viewport->meshNum();
    if(size == 0){
        MainWindow::showMessageBox("Printing", "There is no mesh in the viewport.");
        return;
    }

    QDir* temp = new QDir;
    QString dirName = "TEMP";
    if(!temp->exists(dirName)){
        if(!temp->mkdir(dirName)){
            MainWindow::showMessageBox("Printing", "Failed to create TEMP directory.");
            return;
        }
    }
    delete temp;

    QString stlFileName = dirName+"/temp.stl";
    vector<const aiScene *> scs;
    for (int i=0;i<size;i++){
        scs.push_back(viewport->getModelAt(i)->getMesh()->scene);
    }
    bool successful = AsciiStlExporter(stlFileName.toStdString(), scs);
    if(!successful){
        MainWindow::showMessageBox("Printing", "Failed to generate stl file.");
        QFile::remove(stlFileName);
        return;
    }

    QString gcodeFileName = dirName+"/temp.gcode";
    int perimeter;
    double density, resolution;
    getParameter(perimeter, density, resolution);
    QString slic3rPath = "perl Slic3r_source/slic3r.pl";
    QString configFile = "--load CONFIG/BlackSmithD4.ini";
    QString layerHeight = "--layer-height "+QString::number(resolution,'g',2);
    QString fillDensity = "--fill-density "+QString::number(density,'g',1);
    QString perimeters = "--perimeters "+QString::number(perimeter,10);
    QString outputPath = "--output "+gcodeFileName;
    QStringList cmdLine;
    QString scmd = slic3rPath+" "+configFile+" "+stlFileName+" "+layerHeight+" "+fillDensity+" "+perimeters+" "+outputPath;
    cmdLine << "/c" << scmd;
    gt->getCmd(cmdLine);
    gt->run();
}

void MainWindow::on_left_print_e_2_clicked()
{
    on_left_print_c_1_clicked();
}

//Read gcode from file:
void MainWindow::readGCode(QFile& gfile, QVector<Point>* ps){
    QTextStream in(&gfile);
    QString perline;
    float currentZ = 0.0;
    bool draw = true;
    while(!gfile.atEnd()){
        perline = in.readLine();
        if(perline[0] != QChar('G'))
            continue;

        QStringList list = perline.split(" ",QString::SkipEmptyParts);
        int semicolonIndex = list.indexOf(";");
        if(semicolonIndex != -1){
            int count = list.size() - semicolonIndex;
            for(int i = 0;i < count;i ++)
                list.removeLast();
        }
        if(list.size() <= 1)
            continue;

        float currentX = 0.0;
        float currentY = 0.0;
        float drawE = 0.0;
        if(list[0] == "G92"){
            draw = false;
            continue;
        }
        if(list[1][0] == QChar('Z')){
            currentZ = list[1].remove(0,1).toFloat();
            continue;
        }
        if(list[1][0] != QChar('X'))
            continue;

        if(list[3][0] != QChar('E'))
            draw = false;

        currentX = list[1].remove(0,1).toFloat();
        currentY = list[2].remove(0,1).toFloat();
        drawE = list[3].remove(0,1).toFloat();

        struct Point p;
        p.x = currentX;
        p.y = currentY;
        p.z = currentZ;
        p.draw = draw;
        ps->push_back(p);
        draw = true;
    }
}

//Do preview:
void MainWindow::doPreview(bool isChecked){
    if(isChecked){
        int size = viewport->meshNum();
        if(size == 0){
            MainWindow::showMessageBox("Previewing", "There is no mesh in the viewport.");
            ui->left_preview_c_1->setChecked(false);
            ui->left_preview_e_2->setChecked(false);
            return;
        }
        //Set preview status:
        ui->left_preview_c_1->setChecked(true);
        ui->left_preview_e_2->setChecked(true);

        if(viewport->isChanged()){

            ui->left->setEnabled(false);
            ui->up->setEnabled(false);

            QDir* temp = new QDir;
            QString dirName = "TEMP";
            if(!temp->exists(dirName)){
                if(!temp->mkdir(dirName)){
                    MainWindow::showMessageBox("Previewing", "Failed to create TEMP directory.");
                    ui->left_preview_c_1->setChecked(false);
                    ui->left_preview_e_2->setChecked(false);
                    return;
                }
            }
            delete temp;

            QString stlFileName = dirName+"/temp.stl";
            QString gcodeFileName = dirName+"/temp.gcode";
            //Start exporting process:
            vector<const aiScene *> scs;
            for (int i=0;i<size;i++){
                scs.push_back(viewport->getModelAt(i)->getMesh()->scene);
            }
            bool successful = AsciiStlExporter(stlFileName.toStdString(), scs);
            if(!successful){
                MainWindow::showMessageBox("Previewing", "Failed to generate stl file.");
                ui->left_preview_c_1->setChecked(false);
                ui->left_preview_e_2->setChecked(false);
                QFile::remove(stlFileName);
                QFile::remove(gcodeFileName);
                return;
            }

            //Generate command line:
            int perimeter;
            double density, resolution;
            getParameter(perimeter, density, resolution);

            QString slic3rPath = "perl Slic3r_source/slic3r.pl";
            QString lh = "--layer-height "+QString::number(resolution,'g',2);
            QString fd = "--fill-density "+QString::number(density,'g',1);
            QString pm = "--perimeters "+QString::number(perimeter,10);
            QString outputPath = "--output "+gcodeFileName;
            QString configFile = "--load CONFIG/BlackSmithD4.ini";
            QString scmd = slic3rPath+" "+stlFileName+" "+outputPath+" "+lh+" "+fd+" "+pm+" "+configFile;
            QStringList cmdline;
            cmdline << "/c" << scmd;

            pgt->getCmd(cmdline);
            pgt->run();
            return;

        }
        else{
            //Set preview status:
            viewport->setPreviewStatus(true);
            return;
        }
    }else{
        //Set preview status:
        ui->left_preview_c_1->setChecked(false);
        ui->left_preview_e_2->setChecked(false);

        //Set enabled:
        alignDlg->setEnabled(true);
        scaleDlg->setEnabled(true);
        transDlg->setEnabled(true);
        rotateDlg->setEnabled(true);
        alignDlg->setEnabled(true);
        ui->left_import_c_1->setEnabled(true);
        ui->left_import_e_2->setEnabled(true);
        ui->left_export_c_1->setEnabled(true);
        ui->left_export_e_2->setEnabled(true);
        ui->left_print_c_1->setEnabled(true);
        ui->left_print_e_2->setEnabled(true);
        ui->tabWidget->setEnabled(true);

        //Set preview status:
        viewport->setPreviewStatus(false);
        return;
    }
}

void MainWindow::onDrawPreview()
{
    QString dirName = "TEMP";
    QString stlFileName = dirName+"/temp.stl";
    QString gcodeFileName = dirName+"/temp.gcode";
    //Get gcode file:
    QFile gfile(gcodeFileName);
    if(!gfile.open(QIODevice::ReadOnly | QIODevice::Text)){
        MainWindow::showMessageBox("Previewing", "Failed to read gcode file.");
        ui->left_preview_c_1->setChecked(false);
        ui->left_preview_e_2->setChecked(false);
        QFile::remove(stlFileName);
        QFile::remove(gcodeFileName);
        return;
    }

    //Read gcode file:
    QVector<Point>* ps = new QVector<Point>();
    readGCode(gfile, ps);
    gfile.close();
    viewport->setPoints(ps);
    QFile::remove(stlFileName);
    QFile::remove(gcodeFileName);

    //Set enabled:
    alignDlg->setEnabled(false);
    scaleDlg->setEnabled(false);
    transDlg->setEnabled(false);
    rotateDlg->setEnabled(false);
    alignDlg->setEnabled(false);
    ui->left_import_c_1->setEnabled(false);
    ui->left_import_e_2->setEnabled(false);
    ui->left_export_c_1->setEnabled(false);
    ui->left_export_e_2->setEnabled(false);
    ui->left_print_c_1->setEnabled(false);
    ui->left_print_e_2->setEnabled(false);
    ui->tabWidget->setEnabled(false);
    ui->left->setEnabled(true);
    ui->up->setEnabled(true);

    //Record change:
    viewport->setChanged(false);
    viewport->setPreviewStatus(true);
}

