require 'prefs_base.rb'
require '%{APPNAME}view.rb'

class %{APPNAME} < KDE::XmlGuiWindow
# Default Constructor
  slots 'fileNew()', 'optionsPreferences()', 'setupActions()'
  def initialize()
    super()
    # accept dnd
    setAcceptDrops(true)

    # initialise the view
    #    @view(%{APPNAME}View.new(self)),
    @view = Qt::Widget.new

    # tell the KXmlGuiWindow that this is indeed the main widget
    setCentralWidget(@view)

    # then, setup our actions
    setupActions()

    # add a status bar
    statusBar().show()

    # a call to KXmlGuiWindow::setupGUI() populates the GUI
    # with actions, using KXMLGUI.
    # It also applies the saved mainwindow settings, if any, and ask the
    # mainwindow to automatically save settings if changed: window size,
    # toolbar position, icon size, etc.
    setupGUI()
  end

private
  def fileNew()
    %{APPNAMELC}.new.show()
  end
  def optionsPreferences()
#     dialog = KDE::ConfigDialog.new(self, "settings", Settings::self())
#     generalSettingsDlg = Qt::Widget.new
#     ui_prefs_base = Ui_prefs_base.new
#     ui_prefs_base.setupUi(generalSettingsDlg)
#     dialog.addPage(generalSettingsDlg, KDE::i18n("General"), "package_setting")
#     connect(dialog, SIGNAL('settingsChanged(QString)'), @view, SLOT('settingsChanged()'))
#     dialog.setAttribute( Qt::WA_DeleteOnClose )
#     dialog.show()
  end

private
  def setupActions()
    KDE::StandardAction::openNew(self, SLOT('fileNew()'), actionCollection())
    KDE::StandardAction::quit($kapp, SLOT('closeAllWindows()'), actionCollection())

    KDE::StandardAction::preferences(self, SLOT('optionsPreferences()'), actionCollection())

    # custom menu and menu item - the slot is in the class ${APP_NAME}View
    custom = KDE::Action.new(KDE::Icon.new("colorize"), KDE::i18n("Swi&tch Colors"), self);
    actionCollection().addAction( "switch_action", custom );
    connect(custom, SIGNAL('triggered(bool)'), @view, SLOT('switchColors()'));
  end

end
