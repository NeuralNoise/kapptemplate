echo "Creating $LOCATION_ROOT/${APP_NAME_LC}.h...";
cat << EOF > $LOCATION_ROOT/${APP_NAME_LC}.h
/*
 * ${APP_NAME_LC}.h
 *
 * Copyright (C) 2007 $AUTHOR <$EMAIL>
 */
#ifndef ${APP_NAME_UC}_H
#define ${APP_NAME_UC}_H


#include <kxmlguiwindow.h>

#include "ui_prefs_base.h"

class ${APP_NAME}View;
class QPrinter;
class KToggleAction;
class KUrl;

/**
 * This class serves as the main window for ${APP_NAME}.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author ${AUTHOR} <${EMAIL}>
 * @version ${APP_VERSION}
 */
class ${APP_NAME} : public KXmlGuiWindow
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    ${APP_NAME}();

    /**
     * Default Destructor
     */
    virtual ~${APP_NAME}();

private slots:
    void fileNew();
    void optionsPreferences();

private:
    void setupActions();

private:
    Ui::prefs_base ui_prefs_base ;
    ${APP_NAME}View *m_view;

    QPrinter   *m_printer;
    KToggleAction *m_toolbarAction;
    KToggleAction *m_statusbarAction;
};

#endif // _${APP_NAME_UC}_H_