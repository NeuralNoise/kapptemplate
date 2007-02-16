echo "Creating $LOCATION_ROOT/src/${APP_NAME_LC}pref.cpp...";
cat << EOF > $LOCATION_ROOT/src/${APP_NAME_LC}pref.cpp
#include "${APP_NAME_LC}pref.h"

#include <klocale.h>

#include <Q3Frame>
#include <QHBoxLayout>

#include <qlayout.h>
#include <qlabel.h>

${APP_NAME}Preferences::${APP_NAME}Preferences()
    : KDialog(TreeList, "${APP_NAME} Preferences",
                  Help|Default|Ok|Apply|Cancel, Ok)
{
    // this is the base class for your preferences dialog.  it is now
    // a Treelist dialog.. but there are a number of other
    // possibilities (including Tab, Swallow, and just Plain)
    Q3Frame *frame;
    frame = addPage(i18n("First Page"), i18n("Page One Options"));
    m_pageOne = new ${APP_NAME}PrefPageOne(frame);

    frame = addPage(i18n("Second Page"), i18n("Page Two Options"));
    m_pageTwo = new ${APP_NAME}PrefPageTwo(frame);
}

${APP_NAME}PrefPageOne::${APP_NAME}PrefPageOne(QWidget *parent)
    : Q3Frame(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setAutoAdd(true);

    new QLabel("Add something here", this);
}

${APP_NAME}PrefPageTwo::${APP_NAME}PrefPageTwo(QWidget *parent)
    : Q3Frame(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setAutoAdd(true);

    new QLabel("Add something here", this);
}
#include "${APP_NAME_LC}pref.moc"
