package com.leidos.v2i.config;

import com.leidos.v2i.services.V2IService;
import org.springframework.beans.BeansException;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;

/**
 * Wrapper to always return a reference to the Spring Application Context from
 * within non-Spring enabled beans. Unlike Spring MVC's WebApplicationContextUtils
 * we do not need a reference to the Servlet context for this. All we need is
 * for this bean to be initialized during application startup.
 */
public class V2IApplicationContext implements ApplicationContextAware {

    private static ApplicationContext CONTEXT;
    V2IService service;

    private V2IApplicationContext() {
    }

    private static class V2IApplicationContextHolder  {
        private static final V2IApplicationContext _instance = new V2IApplicationContext();
    }

    public static V2IApplicationContext getInstance()
    {
        return V2IApplicationContextHolder._instance;
    }


    /**
     * This method is called from within the SpeedControl once it is
     * done starting up, it will stick a reference to the app context into this bean.
     * @param context a reference to the ApplicationContext.
     */
    public void setApplicationContext(ApplicationContext context) throws BeansException {
        CONTEXT = context;
    }

    /**
     * get the ApplicationContext
     *
     * @return
     */
    public ApplicationContext getApplicationContext()   {
        return CONTEXT;
    }

    /**
     * Provide access to spring beans through non-spring managed classes
     *
     * @param tClass
     * @param <T>
     * @return
     * @throws org.springframework.beans.BeansException
     */
    public <T>  T getBean(Class<T> tClass) throws BeansException   {
        return CONTEXT.getBean(tClass);
    }

    /**
     * Directly acquire AppConfig
     *
     * @return
     */
    public AppConfig getAppConfig()   {
        return getBean(AppConfig.class);
    }

    public void setService(V2IService service)   {
        this.service = service;
    }

    public V2IService getService()  {
        return service;
    }
}