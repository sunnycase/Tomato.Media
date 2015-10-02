
using System;
using Catel.IoC;
using PinkAlert.Services;

static class ModuleInitializer
{
    public static void Initialize()
    {
        RegisterServices();
    }

    private static void RegisterServices()
    {
        var serviceLocator = IoCConfiguration.DefaultServiceLocator;
        serviceLocator.RegisterType<IThemeService, ThemeService>();
        serviceLocator.RegisterType<ISoundService, SoundService>();
    }
}