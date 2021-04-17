#!/usr/bin/env bash

# Copyright (C) 2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

update_siodb_copyright_years()
{
    local fpath="$1"
    local fname="${fpath##*\/}"
    local fext="${fpath##*\.}"

    if [[ -f "${fpath}" ]] && ( \
            [[ "${fname}" == "Makefile" ]] \
            || [[ "${fext}" == "c" ]] \
            || [[ "${fext}" == "cpp" ]] \
            || [[ "${fext}" == "h" ]] \
            || [[ "${fext}" == "mk" ]] \
            || [[ "${fext}" == "sh" ]] \
            || [[ "${fext}" == "g4" ]] \
        );
    then
        echo "Updating copyright years in ${fpath}"
        local match_count1=$(grep -c -e ".* Copyright .* [[:digit:]]\{4\}-[[:digit:]]\{4\} Siodb" "${fpath}")
        local match_count2=$(grep -c -e ".* Copyright .* [[:digit:]]\{4\} Siodb" "${fpath}")
        if [[ ${match_count1} == 0 && ${match_count2} == 0 ]]; then exit 0; fi

        local total_lines=$(cat "${fpath}" | wc -l)

        if [[ ${match_count1} -gt 0 ]];
        then
            local line_no=$(grep -n -e ".* Copyright .* [[:digit:]]\{4\}-[[:digit:]]\{4\} Siodb" "${fpath}" | head -n1 | cut -f1 -d ':')
            local match=$(head -n${line_no} "${fpath}" | tail -n1 | grep -o -e "[[:digit:]]\{4\}-[[:digit:]]\{4\}")
            local first_year=$(echo "${match}" | cut -f1 -d '-')
            local last_year=${match##*\-}
        else
            local line_no=$(grep -n -e ".* Copyright .* [[:digit:]]\{4\} Siodb" "${fpath}" | head -n1 | cut -f1 -d ':')
            local match=$(head -n${line_no} "${fpath}" | tail -n1 | grep -o -e "[[:digit:]]\{4\}")
            local last_year=${match}
        fi

        local current_year=$(date +%Y)
        if [[ ${last_year} -ne ${current_year} ]];
        then
            if [[ ${match_count1} -gt 0 ]];
            then
                local sed_expr="${line_no}s/${match}/${first_year}-${current_year}/"
            else
                local sed_expr="${line_no}s/${match}/${last_year}-${current_year}/"
            fi
            sed -i -e "${sed_expr}" "${fpath}"
            git add "$fpath"
        fi
    fi
}

case "$1" in
    --about)
        echo "Updated Siodb copyright years on the source and script files"
        ;;
    *)
        for file in `git diff-index --cached --name-only HEAD` ; do
            update_siodb_copyright_years "${file}"
        done
        ;;
esac
