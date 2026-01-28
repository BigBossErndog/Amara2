namespace Amara {
    sol::table parse_csv(sol::state& lua, const std::string& input) {
        sol::table csv_data = lua.create_table();
        sol::table rows = lua.create_table();
        sol::table columns = lua.create_table();
        
        std::istringstream iss(input);
        std::string line;
        int row_number = 1;
        while (std::getline(iss, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (!rows[row_number].valid()) {
                rows[row_number] = lua.create_table();
            }
            
            std::vector<std::string> cells_in_row;
            std::string current_cell;
            bool in_quote = false;

            for (size_t i = 0; i < line.length(); ++i) {
                char ch = line[i];

                if (in_quote) {
                    if (ch == '"') {
                        if (i + 1 < line.length() && line[i + 1] == '"') {
                            current_cell += '"';
                            i++;
                        }
                        else {
                            in_quote = false;
                        }
                    }
                    else {
                        current_cell += ch;
                    }
                }
                else {
                    if (ch == '"') {
                        in_quote = true;
                    }
                    else if (ch == ',') {
                        cells_in_row.push_back(current_cell);
                        current_cell.clear();
                    }
                    else {
                        current_cell += ch;
                    }
                }
            }
            cells_in_row.push_back(current_cell);
            
            int column_number = 1;
            for (const std::string& cell_value : cells_in_row) {
                sol::object cell_obj = json_to_lua(lua, string_to_json(cell_value, false));
                
                rows[row_number][column_number] = cell_obj;
                if (!columns[column_number].valid()) {
                    columns[column_number] = lua.create_table();
                }
                columns[column_number][row_number] = cell_obj;
                column_number += 1;
            }
            row_number += 1;
        }
        
        csv_data["rows"] = rows;
        csv_data["columns"] = columns;
        return csv_data;
    }
}