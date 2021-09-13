/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "BKE_screen.h"

#include "DNA_space_types.h"

#include "ED_screen.h"

#include "RNA_access.h"
#include "RNA_define.h"

#include "WM_api.h"
#include "WM_types.h"

#include "BLI_listbase.h"

#include "MEM_guardedalloc.h"

#include "BKE_context.h"

#include "RNA_access.h"
#include "RNA_define.h"

#include "ED_screen.h"

#include "WM_api.h"
#include "WM_types.h"

#include "space_spreadsheet.cc"
#include "spreadsheet_intern.hh"
#include "spreadsheet_row_filter.hh"
#include <fstream>
#include <string>

using namespace blender::ed::spreadsheet;

static int row_filter_add_exec(bContext *C, wmOperator *UNUSED(op))
{
  SpaceSpreadsheet *sspreadsheet = CTX_wm_space_spreadsheet(C);

  SpreadsheetRowFilter *row_filter = spreadsheet_row_filter_new();
  BLI_addtail(&sspreadsheet->row_filters, row_filter);

  WM_event_add_notifier(C, NC_SPACE | ND_SPACE_SPREADSHEET, sspreadsheet);

  return OPERATOR_FINISHED;
}

static void SPREADSHEET_OT_add_row_filter_rule(wmOperatorType *ot)
{
  ot->name = "Add Row Filter";
  ot->description = "Add a filter to remove rows from the displayed data";
  ot->idname = "SPREADSHEET_OT_add_row_filter_rule";

  ot->exec = row_filter_add_exec;
  ot->poll = ED_operator_spreadsheet_active;

  ot->flag = OPTYPE_REGISTER | OPTYPE_UNDO;
}

static int row_filter_remove_exec(bContext *C, wmOperator *op)
{
  SpaceSpreadsheet *sspreadsheet = CTX_wm_space_spreadsheet(C);

  SpreadsheetRowFilter *row_filter = (SpreadsheetRowFilter *)BLI_findlink(
      &sspreadsheet->row_filters, RNA_int_get(op->ptr, "index"));
  if (row_filter == nullptr) {
    return OPERATOR_CANCELLED;
  }

  BLI_remlink(&sspreadsheet->row_filters, row_filter);
  spreadsheet_row_filter_free(row_filter);

  WM_event_add_notifier(C, NC_SPACE | ND_SPACE_SPREADSHEET, sspreadsheet);

  return OPERATOR_FINISHED;
}

static void SPREADSHEET_OT_remove_row_filter_rule(wmOperatorType *ot)
{
  ot->name = "Remove Row Filter";
  ot->description = "Remove a row filter from the rules";
  ot->idname = "SPREADSHEET_OT_remove_row_filter_rule";

  ot->exec = row_filter_remove_exec;
  ot->poll = ED_operator_spreadsheet_active;

  ot->flag = OPTYPE_REGISTER | OPTYPE_UNDO;

  RNA_def_int(ot->srna, "index", 0, 0, INT_MAX, "Index", "", 0, INT_MAX);
}

static int export_as_csv_exec(bContext *C, wmOperator *op)
{
  std::unique_ptr<DataSource> data_source = get_data_source(C);
  SpaceSpreadsheet *sspreadsheet = CTX_wm_space_spreadsheet(C);
  ResourceScope scope;
  if (!data_source) {
    data_source = std::make_unique<DataSource>();
  }
  const int row_size = data_source->tot_rows();
  std::cout << "Row size" << std::endl;
  std::cout << row_size;
  std::vector<std::string> dataset(row_size + 1, "");

  LISTBASE_FOREACH (SpreadsheetColumn *, column, &sspreadsheet->columns) {
    std::unique_ptr<ColumnValues> values_ptr = data_source->get_column_values(*column->id);
    // BLI_assert(values_ptr);
    const ColumnValues *values = scope.add(std::move(values_ptr), __func__);

    std::cout << "Col name" << values->name() << std::endl;

    CellValue cell_value;
    int tot_values = values->size();
    std::string col_name = values->name();
    // TODO: Append extra commas for float3, float2 et al.
    dataset[0] += (col_name) + ",";

    for (int i = 0; i < row_size; i++) {
      std::string value_str = "";
      values->get_value(i, cell_value);
      if (cell_value.value_float3.has_value()) {
        const float3 value = *cell_value.value_float3;
        for (int j = 0; j < 3; j++) {
          value_str += std::to_string(value[j]) + ",";
        }
        // std::cout << value_str << std::endl;
      }
      else if (cell_value.value_float.has_value()) {
        const float value = *cell_value.value_float;
        value_str += std::to_string(value) + ",";
      }
      else if (cell_value.value_int.has_value()) {
        const int value = *cell_value.value_int;
        value_str += std::to_string(value) + ",";
      }
      else if (cell_value.value_bool.has_value()) {
        const bool value = *cell_value.value_bool;
        value_str += std::to_string(value) + ",";
      }
      dataset[i + 1] += value_str;
    }

    // Create a file and write to it.

    std::string file_name = "C:/users/himan/Desktop/dataset_blender.csv";

    std::ofstream ost{file_name};
    if (!ost) {
      std::cout << "can't open file";
      return OPERATOR_CANCELLED;
    }


    for (int i = 0; i < row_size; i++) {
      ost << dataset[i] << "\n";
    }
  }

  return OPERATOR_FINISHED;
}

static void SPREADSHEET_OT_export_as_csv(wmOperatorType *ot)
{
  ot->name = "Export as CSV";
  ot->description = "Export Spreadsheet data into CSV";
  ot->idname = "SPREADSHEET_OT_export_as_csv";

  ot->exec = export_as_csv_exec;
  ot->poll = ED_operator_spreadsheet_active;
}

static int select_component_domain_invoke(bContext *C,
                                          wmOperator *op,
                                          const wmEvent *UNUSED(event))
{
  GeometryComponentType component_type = static_cast<GeometryComponentType>(
      RNA_int_get(op->ptr, "component_type"));
  AttributeDomain attribute_domain = static_cast<AttributeDomain>(
      RNA_int_get(op->ptr, "attribute_domain_type"));

  SpaceSpreadsheet *sspreadsheet = CTX_wm_space_spreadsheet(C);
  sspreadsheet->geometry_component_type = component_type;
  sspreadsheet->attribute_domain = attribute_domain;

  /* Refresh header and main region. */
  WM_main_add_notifier(NC_SPACE | ND_SPACE_SPREADSHEET, nullptr);

  return OPERATOR_FINISHED;
}

static void SPREADSHEET_OT_change_spreadsheet_data_source(wmOperatorType *ot)
{
  ot->name = "Change Visible Data Source";
  ot->description = "Change visible data source in the spreadsheet";
  ot->idname = "SPREADSHEET_OT_change_spreadsheet_data_source";

  ot->invoke = select_component_domain_invoke;

  RNA_def_int(ot->srna, "component_type", 0, 0, INT16_MAX, "Component Type", "", 0, INT16_MAX);
  RNA_def_int(ot->srna,
              "attribute_domain_type",
              0,
              0,
              INT16_MAX,
              "Attribute Domain Type",
              "",
              0,
              INT16_MAX);

  ot->flag = OPTYPE_INTERNAL;
}

void spreadsheet_operatortypes()
{
  WM_operatortype_append(SPREADSHEET_OT_add_row_filter_rule);
  WM_operatortype_append(SPREADSHEET_OT_remove_row_filter_rule);
  WM_operatortype_append(SPREADSHEET_OT_change_spreadsheet_data_source);
  WM_operatortype_append(SPREADSHEET_OT_export_as_csv);
}
