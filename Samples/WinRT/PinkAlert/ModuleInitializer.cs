
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
        IoCConfiguration.DefaultServiceLocator.RegisterType<IThemeService, ThemeService>();
    }
}