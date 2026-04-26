#include "mtgscanner.h"

namespace MTGS {

static MTGScanner::Theme globalTheme = MTGScanner::Theme::Dark;

bool MTGScanner::parseControlsConfig(const QString &configPath)
{
    // Placeholder for future implementation
    // This function would read a JSON or INI file to set globalTheme and possibly other settings.
    // QSettings settings(":/qtquickcontrols2.conf", QSettings::IniFormat);
    // settings.beginGroup("MyStyle");
    // QString accent = settings.value("Accent").toString();

    return false;
}

MTGScanner::MTGScanner(QObject *parent)
    : QQuickAttachedPropertyPropagator(parent)
    , m_theme(Theme::Dark) // Default to Dark theme
{
    // A static function could be called here that reads globalTheme from a
    // settings file once at startup. That value would override the global
    // value. This is similar to what the Imagine and Material styles do, for
    // example.
    // parseControlsConfig(":/qtquickcontrols2.conf");

    initialize();
}

MTGScanner::~MTGScanner()
{}

MTGScanner *MTGScanner::qmlAttachedProperties(QObject *object)
{
    return new MTGScanner(object);
}

MTGScanner::Theme MTGScanner::theme() const
{
    return m_theme;
}

QColor MTGScanner::accentColor() const
{
    return m_theme == Theme::Light ? QColor("#5C7BA6") : QColor("#8BA3C7");
}

QColor MTGScanner::primaryColor() const
{
    return m_theme == Theme::Light ? QColor("#5C7BA6") : QColor("#8BA3C7");
}

QColor MTGScanner::backgroundColor() const
{
    return m_theme == Theme::Light ? QColor("#F4F4F6") : QColor("#1A1A1A");
}

QColor MTGScanner::foregroundColor() const
{
    return m_theme == Theme::Light ? QColor("#1E1E2A") : QColor("#E5E2E1");
}

QColor MTGScanner::alternateBackgroundColor() const
{
    return m_theme == Theme::Light ? QColor("#FFFFFF") : QColor("#1E1E1E");
}

QColor MTGScanner::surfaceColor() const
{
    return m_theme == Theme::Light ? QColor("#FFFFFF") : QColor("#1E1E1E");
}

QColor MTGScanner::surfaceTextColor() const
{
    return m_theme == Theme::Light ? QColor("#1E1E2A") : QColor("#E5E2E1");
}

QColor MTGScanner::separatorColor() const
{
    return m_theme == Theme::Light ? QColor("#E0E0E5") : QColor("#252525");
}

QColor MTGScanner::hoverColor() const
{
    return m_theme == Theme::Light ? QColor("#F0F0F3") : QColor("#252525");
}

QColor MTGScanner::successColor() const
{
    return m_theme == Theme::Light ? QColor("#2E7D32") : QColor("#4CAF50");
}

QColor MTGScanner::warningColor() const
{
    return m_theme == Theme::Light ? QColor("#E65100") : QColor("#F59E0B");
}

QColor MTGScanner::errorColor() const
{
    return m_theme == Theme::Light ? QColor("#C62828") : QColor("#FFB4AB");
}

QColor MTGScanner::buttonColor() const
{
    return m_theme == Theme::Light ? QColor("#5C7BA6") : QColor("#8BA3C7");
}

QColor MTGScanner::buttonTextColor() const
{
    return m_theme == Theme::Light ? QColor("#FFFFFF") : QColor("#1A1A1A");
}

QColor MTGScanner::disabledColor() const
{
    return m_theme == Theme::Light ? QColor("#FAFAFC") : QColor("#1C1C1C");
}

QColor MTGScanner::disabledTextColor() const
{
    return m_theme == Theme::Light ? QColor("#A0A0AB") : QColor("#959393");
}

QColor MTGScanner::placeholderTextColor() const
{
    return m_theme == Theme::Light ? QColor("#8B8B96") : QColor("#8E9198");
}

QColor MTGScanner::overlayColor() const
{
    return m_theme == Theme::Light ? QColor("#F4F4F6E6") : QColor("#1A1A1ACC");
}

QColor MTGScanner::popupBorderColor() const
{
    return m_theme == Theme::Light ? QColor("#E0E0E5") : QColor("#252525");
}

void MTGScanner::setTheme(Theme theme)
{
    m_explicitTheme = true;
    if (m_theme == theme)
        return;

    m_theme = theme;
    propagateTheme();
    emit themeChanged();
}

void MTGScanner::resetTheme()
{
    if (!m_explicitTheme)
        return;

    m_explicitTheme = false;
    MTGScanner *mystyle = qobject_cast<MTGScanner *>(attachedParent());
    inheritTheme(mystyle ? mystyle->theme() : globalTheme);
}

void MTGScanner::inheritTheme(Theme theme)
{
    if (m_explicitTheme || m_theme == theme)
        return;

    m_theme = theme;
    propagateTheme();
    emit themeChanged();
}

void MTGScanner::propagateTheme()
{
    const auto children = attachedChildren();
    for (auto *child : children) {
        if (auto *childScanner = qobject_cast<MTGScanner *>(child)) {
            childScanner->inheritTheme(m_theme);
        }
    }
}

void MTGScanner::attachedParentChange(QQuickAttachedPropertyPropagator *newParent, QQuickAttachedPropertyPropagator *oldParent)
{
    Q_UNUSED(oldParent);
    
    if (m_explicitTheme) {
        // If this scanner has an explicit theme set, do not inherit from the new parent
        return;
    }

    if (auto *parentScanner = qobject_cast<MTGScanner *>(newParent)) {
        inheritTheme(parentScanner->theme());
    }
}

} // namespace MTGS