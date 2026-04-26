#pragma once

#include <QColor>
#include <QQmlEngine>
#include <QQuickAttachedPropertyPropagator>

#include "mtgscanner_export.h"

namespace MTGS {

/**
 * @brief MTGScanner is a QML attached property class that provides a comprehensive theming system 
 *      for the MTGScanner application. It allows components to inherit theme properties from their 
 *      parent or set their own theme explicitly. The class defines a set of color properties that 
 *      adapt based on the current theme (Light or Dark), enabling consistent styling across the application.
*/
class MTGSCANNER_EXPORT MTGScanner : public QQuickAttachedPropertyPropagator
{
    Q_OBJECT

    // Core Theme Mode
    Q_PROPERTY(Theme theme READ theme WRITE setTheme RESET resetTheme NOTIFY themeChanged FINAL)

    // Global Colors
    Q_PROPERTY(QColor accentColor READ accentColor NOTIFY themeChanged FINAL)
    Q_PROPERTY(QColor primaryColor READ primaryColor NOTIFY themeChanged FINAL)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor NOTIFY themeChanged FINAL)
    Q_PROPERTY(QColor foregroundColor READ foregroundColor NOTIFY themeChanged FINAL)
    Q_PROPERTY(QColor alternateBackgroundColor READ alternateBackgroundColor NOTIFY themeChanged FINAL)

    // Container Specifics (Sidebar/Drawer/Cards)
    Q_PROPERTY(QColor surfaceColor READ surfaceColor NOTIFY themeChanged FINAL)
    Q_PROPERTY(QColor surfaceTextColor READ surfaceTextColor NOTIFY themeChanged FINAL)
    Q_PROPERTY(QColor separatorColor READ separatorColor NOTIFY themeChanged FINAL)
    Q_PROPERTY(QColor hoverColor READ hoverColor NOTIFY themeChanged FINAL)
    Q_PROPERTY(QColor successColor READ successColor NOTIFY themeChanged FINAL)
    Q_PROPERTY(QColor warningColor READ warningColor NOTIFY themeChanged FINAL)
    Q_PROPERTY(QColor errorColor   READ errorColor   NOTIFY themeChanged FINAL)
    
    // Control Specifics
    Q_PROPERTY(QColor buttonColor READ buttonColor NOTIFY themeChanged FINAL)
    Q_PROPERTY(QColor buttonTextColor READ buttonTextColor NOTIFY themeChanged FINAL)
    Q_PROPERTY(QColor disabledColor READ disabledColor NOTIFY themeChanged FINAL)
    Q_PROPERTY(QColor disabledTextColor READ disabledTextColor NOTIFY themeChanged FINAL)
    Q_PROPERTY(QColor placeholderTextColor READ placeholderTextColor NOTIFY themeChanged FINAL)

    // Utility
    Q_PROPERTY(QColor overlayColor READ overlayColor NOTIFY themeChanged FINAL)
    Q_PROPERTY(QColor popupBorderColor READ popupBorderColor NOTIFY themeChanged FINAL)


    QML_ELEMENT
    QML_ATTACHED(MTGScanner)
    QML_UNCREATABLE("MTGScanner is an attached property and cannot be created directly")
    QML_ADDED_IN_VERSION(1, 0)
public:
    enum Theme {
        Light,
        Dark
    };
    Q_ENUM(Theme)

    explicit MTGScanner(QObject *parent = nullptr);
    ~MTGScanner();
    static MTGScanner *qmlAttachedProperties(QObject *object);

    Theme theme() const;
    QColor accentColor() const;
    QColor primaryColor() const;
    QColor backgroundColor() const;
    QColor foregroundColor() const;
    QColor alternateBackgroundColor() const;
    QColor surfaceColor() const;
    QColor surfaceTextColor() const;
    QColor separatorColor() const;
    QColor hoverColor() const;
    QColor successColor() const;
    QColor warningColor() const;
    QColor errorColor() const;
    QColor buttonColor() const;
    QColor buttonTextColor() const;
    QColor disabledColor() const;
    QColor disabledTextColor() const;
    QColor placeholderTextColor() const;
    QColor overlayColor() const;
    QColor popupBorderColor() const;
    
public slots:
    void setTheme(Theme theme);
    void resetTheme();
    void inheritTheme(Theme theme);
    void propagateTheme();

signals:
    void themeChanged();

protected:
    void attachedParentChange(QQuickAttachedPropertyPropagator *newParent, QQuickAttachedPropertyPropagator *oldParent) override;
    static bool parseControlsConfig(const QString &configPath);

private:
    Theme m_theme;
    bool m_explicitTheme = false;
};

} // namespace MTGS