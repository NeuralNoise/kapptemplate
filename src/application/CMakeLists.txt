set(kapptemplate_SRCS
   kapptemplate.cpp
   main.cpp
   apptemplatesmodel.cpp
   apptemplateitem.cpp
   generatepage.cpp
   choicepage.cpp
)

ki18n_wrap_ui(kapptemplate_SRCS introduction.ui choice.ui properties.ui generate.ui)

kconfig_add_kcfg_files(kapptemplate_SRCS prefs.kcfgc)

add_executable(kapptemplate ${kapptemplate_SRCS})

target_link_libraries(kapptemplate
    KF5::CoreAddons
    KF5::ConfigWidgets
    KF5::Completion
    KF5::KIOWidgets
    KF5::Archive
    KF5::KIOCore
    KF5::I18n
    KF5::NewStuff
)

install(TARGETS kapptemplate ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.kapptemplate.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR})
install(FILES org.kde.kapptemplate.appdata.xml DESTINATION  ${KDE_INSTALL_METAINFODIR})
install(FILES kapptemplate.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES kapptemplate.knsrc  DESTINATION  ${KDE_INSTALL_CONFDIR})
