#ifndef _ABSTRACTMODULE_H_
#define _ABSTRACTMODULE_H_

/**
 * Abstract base class for modules which support setup and reentrant loop methods.
 */
class AbstractModule {
    public:
      virtual void setup(void) = 0;
      virtual void loop(void) = 0;
};

#endif