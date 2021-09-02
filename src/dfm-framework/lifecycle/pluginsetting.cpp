#include "pluginsetting.h"
#include "private/pluginmetaobject_p.h"

DPF_USE_NAMESPACE

PluginSetting::PluginSetting(QSettings::Scope scope, const QString &organization,
                             const QString &application, QObject *parent)
    : QSettings(scope,organization,application,parent)
{

}

void PluginSetting::setPluginEnable(const PluginMetaObject &meta, bool enabled)
{
    beginGroup(meta.name());
    setValue(PLUGIN_VERSION, meta.version());
    setValue(ENABLED, enabled);
    endGroup();
}