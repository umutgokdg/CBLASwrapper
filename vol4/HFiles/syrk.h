/* ************************************************************************
 * Copyright 2013 Advanced Micro Devices, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ************************************************************************/


#ifndef KTEST_SYRK_H__
#define KTEST_SYRK_H__

#include "step.h"

namespace clMath {

class SyrkStep : public Step {
public:
    SyrkStep(cl_device_id device);
    SyrkStep(ListNode *node);

    virtual void fixLD();
    virtual void declareVars(Step *masterStep);
};

}   // namespace clMath

#endif  // KTEST_SYRK_H__
