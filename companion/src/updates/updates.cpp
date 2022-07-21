/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "updates.h"
#include "updateinterface.h"
#include "updatesdialog.h"
#include "progressdialog.h"
#include "progresswidget.h"
#include "appdata.h"
#include "helpers.h"

#include <QMessageBox>

Updates::Updates(QWidget * parent, UpdateFactories * updateFactories) :
  QWidget(parent),
  factories(updateFactories)
{
}

Updates::~Updates()
{
}

void Updates::autoUpdates()
{
  if (g.updateCheckFreq() == AppData::UPDATE_CHECK_MANUAL)
    return;

  if (g.lastUpdateCheck().trimmed().isEmpty())
    g.lastUpdateCheck(QDateTime::currentDateTime().addDays(-60).toString(Qt::ISODate));

  QDateTime dt = QDateTime::fromString(g.lastUpdateCheck(), Qt::ISODate);

  if (g.updateCheckFreq() == AppData::UPDATE_CHECK_DAILY)
    dt = dt.addDays(1);
  else if (g.updateCheckFreq() == AppData::UPDATE_CHECK_WEEKLY)
    dt = dt.addDays(7);
  else if (g.updateCheckFreq() == AppData::UPDATE_CHECK_MONTHLY)
    dt = dt.addDays(30);

  if (dt > QDateTime::currentDateTime().toLocalTime()) {
    qDebug() << "Update next due:" << dt.toString(Qt::ISODate);
    return;
  }

  g.lastUpdateCheck(QDateTime::currentDateTime().toString(Qt::ISODate));

  if (!factories->updatesAvailable())
    return;

  if (QMessageBox::question(parentWidget(), CPN_STR_APP_NAME % ": " % tr("Checking for Updates"),
                            tr("One or more updates are available. Update now?"),
                            (QMessageBox::Yes | QMessageBox::No), QMessageBox::No) == QMessageBox::Yes) {

    factories->resetAllRunEnvironments();

    factories->autoUpdate();
  }
}

void Updates::manualUpdates()
{
  factories->resetAllRunEnvironments();

  UpdatesDialog *dlg = new UpdatesDialog(this, factories);

  if (dlg->exec()) {
    ProgressDialog progressDialog(this, tr("Update Components"), CompanionIcon("fuses.png"), true);
    progressDialog.progress()->lock(true);
    progressDialog.progress()->setInfo(tr("Starting..."));
    bool ok = factories->manualUpdate(progressDialog.progress());
    progressDialog.progress()->lock(false);
    progressDialog.progress()->setInfo(tr("Finished %1").arg(ok ? tr("successfully") : tr("with errors")));
    progressDialog.exec();
  }

  delete dlg;
}