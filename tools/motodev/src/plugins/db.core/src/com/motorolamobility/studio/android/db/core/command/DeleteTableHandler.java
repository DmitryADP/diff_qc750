/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.motorolamobility.studio.android.db.core.command;

import org.eclipse.core.commands.AbstractHandler;
import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.commands.IHandler;
import org.eclipse.core.runtime.IStatus;

import com.motorola.studio.android.common.utilities.EclipseUtils;
import com.motorolamobility.studio.android.db.core.i18n.DbCoreNLS;
import com.motorolamobility.studio.android.db.core.ui.IDbNode;
import com.motorolamobility.studio.android.db.core.ui.ITableNode;
import com.motorolamobility.studio.android.db.core.ui.ITreeNode;

public class DeleteTableHandler extends AbstractHandler implements IHandler
{

    private ITreeNode node;

    public DeleteTableHandler()
    {
    }

    public DeleteTableHandler(ITreeNode node)
    {
        this.node = node;
    }

    public Object execute(ExecutionEvent event) throws ExecutionException
    {
        if (node instanceof ITableNode)
        {
            ITableNode tableNode = (ITableNode) node;
            IDbNode dbNode = (IDbNode) tableNode.getParent();

            boolean shouldProceed =
                    EclipseUtils
                            .showQuestionDialog(
                                    DbCoreNLS.DeleteTableHandler_ConfirmationQuestionDialog_Title,
                                    DbCoreNLS
                                            .bind(DbCoreNLS.DeleteTableHandler_ConfirmationQuestionDialog_Description,
                                                    node.getName()));

            if (shouldProceed)
            {
                IStatus status = dbNode.deleteTable(tableNode);

                if ((status != null) && !status.isOK())
                {
                    //something went wrong when deleting the table...
                    EclipseUtils.showErrorDialog(DbCoreNLS.DbNode_CouldNotDeleteTable,
                            status.getMessage());
                }

            }
        }

        return null;
    }
}
