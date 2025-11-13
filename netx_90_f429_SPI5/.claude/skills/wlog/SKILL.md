---
name: wlog
description: Create detailed work logs in Markdown format with timestamp-prefixed filenames. Use when the user requests to document work activities, create work logs, record task progress, track project activities, write daily reports, or maintain project documentation. Supports both simple and comprehensive log formats with structured sections for tasks, issues, decisions, and next steps.
---

# W Logger

Create comprehensive, timestamped work logs in Markdown format to document work activities, track progress, and maintain project history.

## Core Functionality

Generate work log files with automatic timestamp-based naming (format: `YYYYMMDD_HHMMSS_work_log.md`) containing structured documentation of:
- Work overview and objectives
- Tasks performed and progress status
- Issues encountered and resolutions
- Key decisions and their rationale
- Collaboration notes and communication
- Next steps and pending items

## Log Creation Workflow

### 1. Determine Log Type
Ask the user which template to use:
- **Simple**: Basic structure for quick daily logs (use `assets/template_simple.md`)
- **Detailed**: Comprehensive structure for complex projects (use `assets/template_detailed.md`)
- **Custom**: User provides specific structure or requirements

### 2. Gather Information
Collect key details through conversation:
- Date and project context
- Tasks completed and in progress
- Any issues or blockers encountered
- Important decisions made
- Next planned actions

### 3. Generate Content
Structure the log content based on the chosen template, filling in all relevant sections with the information provided. Ensure:
- Clear, descriptive headings
- Bullet points for lists
- Checkboxes for task status
- Proper formatting for readability
- Specific details rather than vague statements

### 4. Create File
Use the `scripts/create_log.py` script to generate the timestamped file:

```bash
python scripts/create_log.py "<markdown_content>" <output_directory> [prefix] [date_format]
```

Parameters:
- `<markdown_content>`: Full log content (required)
- `<output_directory>`: Target directory (default: current directory)
- `[prefix]`: Filename prefix (default: "work_log")
- `[date_format]`: Timestamp format (default: "%Y%m%d_%H%M%S")

**Example**:
```bash
python scripts/create_log.py "# Daily Log\n\n## Tasks\n- Completed API integration" ./logs project_alpha
```

### 5. Provide File Link
After creating the file, copy it to `/mnt/user-data/outputs` and provide a download link to the user using the `computer://` protocol.

## Template Selection Guide

### Use Simple Template When:
- Quick daily updates
- Straightforward tasks
- Minimal complexity
- Personal progress tracking

### Use Detailed Template When:
- Complex project work
- Multiple stakeholders
- Critical decisions made
- Significant issues resolved
- Comprehensive documentation needed
- Team collaboration required

## Best Practices

1. **Be Specific**: Include concrete details, metrics, and outcomes rather than vague descriptions
2. **Record Issues**: Document problems and solutions for future reference
3. **Track Decisions**: Explain why decisions were made and their expected impact
4. **Link Resources**: Include URLs to related documents, code, or tools
5. **Update Regularly**: Create logs consistently for accurate project history
6. **Use Checkboxes**: Track completion status with `[x]` and `[ ]` markers

## Additional Resources

For detailed guidance on writing effective work logs, refer to `references/guidelines.md` which includes:
- Purpose and benefits of work logging
- Section-by-section writing tips
- Common pitfalls to avoid
- File organization recommendations

## Examples

### Quick Daily Log
```markdown
# Daily Work Log

## Tasks Completed
- Fixed authentication bug in user login
- Reviewed PR #234
- Updated API documentation

## Next Steps
- Deploy to staging environment
- Schedule code review meeting
```

### Comprehensive Project Log
```markdown
# Project Alpha - Sprint 3 Work Log

## 📋 Overview
- Date: 2025-11-02
- Sprint: 3 of 4
- Team: Backend Development

## ✅ Completed Tasks
- [x] Database schema migration
- [x] API endpoint implementation
- [x] Unit test coverage increased to 85%

## 🐛 Issues Resolved
### Database Connection Timeout
- **Root Cause**: Connection pool size too small
- **Solution**: Increased pool size from 10 to 50
- **Impact**: Response time improved by 40%

## 💡 Key Decisions
- Adopted Redis for session caching
- Reason: Reduce database load and improve performance
- Impact: Expected 30% reduction in DB queries
```

## Resources

### scripts/
Contains `create_log.py` - Python script for generating timestamped work log files with proper naming conventions.

### references/
Contains `guidelines.md` - Comprehensive guide for writing effective work logs with best practices and tips.

### assets/
Contains work log templates:
- `template_simple.md` - Basic template for quick daily logs
- `template_detailed.md` - Comprehensive template for detailed project documentation
