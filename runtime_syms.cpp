#include "ast.hpp"

/* Runtime Library Function Formal Parameters */
/* 1. IO funs */
Ret_type rNothing(nullptr);
Ret_type rInt(new Data_type(INT));
Ret_type rChar(new Data_type(CHAR));

Fpar_def_list writeInteger_pars(
	new Fpar_def(
		false,
		new Id_list(new Id("n")),
		new Fpar_type(
			new Data_type(INT),
			false,
			nullptr
		)
	)
);

Fpar_def_list writeChar_pars(
	new Fpar_def(
		false,
		new Id_list(new Id("c")),
		new Fpar_type(
			new Data_type(CHAR),
			false,
			nullptr
		)
	)
);

Fpar_def_list writeString_pars(
	new Fpar_def(
		true,
		new Id_list(new Id("s")),
		new Fpar_type(
			new Data_type(CHAR),
			true,
			nullptr
		)
	)
);

/* readInteger has no formal pars so a nullptr will be used in symbol construction */
/* readChar    has no formal pars so a nullptr will be used in symbol construction */

Fpar_def_list readString_pars(
	new Fpar_def(
		false,
		new Id_list(new Id("n")),
		new Fpar_type(
			new Data_type(INT),
			false,
			nullptr
		)
	)
);

/* 2. Conversion funs */
Fpar_def_list ascii_pars(
	new Fpar_def(
		false,
		new Id_list(new Id("c")),
		new Fpar_type(
			new Data_type(CHAR),
			false,
			nullptr
		)
	)
);

Fpar_def_list chr_pars(
	new Fpar_def(
		false,
		new Id_list(new Id("n")),
		new Fpar_type(
			new Data_type(INT),
			false,
			nullptr
		)
	)
);

/* 3. String funs */
Fpar_def_list strlen_pars(
	new Fpar_def(
		true,
		new Id_list(new Id("s")),
		new Fpar_type(
			new Data_type(CHAR),
			true,
			nullptr
		)
	)
);

Id_list strcmp_id_list(new Id("s1"));
Fpar_def_list strcmp_pars(
	new Fpar_def(
		true,
		&strcmp_id_list,
		new Fpar_type(
			new Data_type(CHAR),
			true,
			nullptr
		)
	)
);

Id_list strcpy_id_list(new Id("trg"));
Fpar_def_list strcpy_pars(
	new Fpar_def(
		true,
		&strcpy_id_list,
		new Fpar_type(
			new Data_type(CHAR),
			true,
			nullptr
		)
	)
);

Id_list strcat_id_list(new Id("trg"));
Fpar_def_list strcat_pars(
	new Fpar_def(
		true,
		&strcat_id_list,
		new Fpar_type(
			new Data_type(CHAR),
			true,
			nullptr
		)
	)
);

void finish_runtime_syms() { // needed by symbol_table for runtime lib formal params
	readString_pars.append(
		new Fpar_def(
			true,
			new Id_list(new Id("s")),
			new Fpar_type(
				new Data_type(CHAR),
				true,
				nullptr
			)
		)
	);

	strcmp_id_list.append(new Id("s2"));
	strcpy_id_list.append(new Id("src"));
	strcat_id_list.append(new Id("src"));
}
