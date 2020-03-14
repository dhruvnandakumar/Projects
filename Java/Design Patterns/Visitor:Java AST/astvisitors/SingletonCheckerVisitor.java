package ecs160.visitor.astvisitors;

import ecs160.visitor.utilities.ASTNodeTypePrinter;
import ecs160.visitor.utilities.UtilReader;
import org.eclipse.jdt.core.JavaCore;
import org.eclipse.jdt.core.dom.*;
import org.jetbrains.annotations.NotNull;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.List;
import java.util.Map;


public class SingletonCheckerVisitor extends ASTVisitor {
    private String className;
    private boolean privConst;
    private boolean instanceOfClassType;
    private boolean psiVar;
    private boolean classInstCreat;
    private int instanceCreationCount;
    private boolean insideIf;

    private SingletonCheckerVisitor(String name) {
        this.className = name;
        privConst = false;
        instanceOfClassType = false;
        psiVar = false;
        classInstCreat = false;
        instanceCreationCount = 0;
        insideIf = false;
    }

    @NotNull
    public static SingletonCheckerVisitor setUpGrader(String sourceFile, String className) {

        File file = new File(sourceFile);
        String text = "";
        try {
            text = UtilReader.read(file);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }

        ASTParser parser = ASTParser.newParser(AST.JLS12); //Create a parser for a version of the Java language (12 here)
        Map<String, String> options = JavaCore.getOptions(); //get the options for a type of Eclipse plugin that is the basis of Java plugins
        options.put(JavaCore.COMPILER_SOURCE, JavaCore.VERSION_12); //Specify that we are on Java 12 and add it to the options...
        parser.setCompilerOptions(options); //forward all these options to our parser
        parser.setKind(ASTParser.K_COMPILATION_UNIT); //What kind of constructions will be parsed by this parser.  K_COMPILATION_UNIT means we are parsing whole files.
        parser.setResolveBindings(true); //Enable looking for bindings/connections from this file to other parts of the program.
        parser.setBindingsRecovery(true); //Also attempt to recover incomplete bindings (only can be set to true if above line is set to true).
        String[] classpath = {System.getProperty("java.home") + "/lib/rt.jar"}; //Link to your Java installation.
        parser.setEnvironment(classpath, new String[]{""}, new String[]{"UTF-8"}, true);
        parser.setSource(text.toCharArray()); //Load in the text of the file to parse.
        parser.setUnitName(file.getAbsolutePath()); //Load in the absolute path of the file to parse
        CompilationUnit compUnit = (CompilationUnit) parser.createAST(null); //Create the tree and link to the root node.

        SingletonCheckerVisitor checker = new SingletonCheckerVisitor(className);

        compUnit.accept(checker);

        return checker;

    }

    public boolean visit(TypeDeclaration node) {
        boolean scanChildren = true;

        String actualName = node.getName().getIdentifier();

        if (!actualName.equals(this.className)) {
            scanChildren = false;
        }

        return scanChildren;
    }

    public boolean visit(FieldDeclaration node) {

        int modifiers = node.getModifiers();

        boolean isPrivate = Modifier.isPrivate(modifiers);
        boolean isStatic = Modifier.isStatic(modifiers);

        String fieldName = "N/A";
        boolean actuallyFound = false;

        if (node.getType().isSimpleType()) {
            if (((SimpleType) node.getType()).getName().isSimpleName()) {
                actuallyFound = true;
                fieldName = ((SimpleName) ((SimpleType) node.getType()).getName()).getIdentifier();
            }
        }

        if (actuallyFound && isPrivate && isStatic && fieldName.equals(className))
            psiVar = true;

        return false;

    }

    public boolean visit(MethodDeclaration node) {

        if (node.isConstructor()) {
            int modifiers = node.getModifiers();
            if (Modifier.isPrivate(modifiers)) {
                privConst = true;
            }
            return false;
        }

        int modifiers = node.getModifiers();

        boolean isPublic = Modifier.isPublic(modifiers);
        boolean isStatic = Modifier.isStatic(modifiers);
        boolean returnsClass = false;

        Type returnType = node.getReturnType2();

        if (returnType.isSimpleType()) {
            String actualReturn = ((SimpleName) ((SimpleType) returnType).getName()).getIdentifier();

            if (actualReturn.equals(className)) {
                returnsClass = true;
            }
        }

        if (isPublic && isStatic && returnsClass)
            instanceOfClassType = true;

        if (instanceOfClassType) {

            Block functionBody = node.getBody();
            List<Statement> functionStatements = functionBody.statements();

            for (Statement statement : functionStatements) {
                    statement.accept(this);
            }

        }

        if (instanceCreationCount != 1)
            classInstCreat = false;


        return false;

    }

    public boolean visit(IfStatement node) {

        Block thenStmt = (Block) node.getThenStatement();
        Block elseStmt = (Block) node.getElseStatement();

        List<Statement> thenStmts = thenStmt.statements();
        List<Statement> elseStmts = elseStmt.statements();

        insideIf = true;

        for(Statement stmt : thenStmts){
            stmt.accept(this);
        }

        for(Statement stmt : elseStmts){
            stmt.accept(this);
        }

        insideIf = false;

        return false;
    }

    public boolean visit(ClassInstanceCreation node) {

        SimpleType instType = (SimpleType) node.getType();
        if (((SimpleName) instType.getName()).getIdentifier().equals(className)) {

            if(insideIf)
                classInstCreat = true;

            instanceCreationCount++;
        }

        return false;
    }

    public boolean gradeA() {
        return privConst;
    }

    public boolean gradeB() {
        return instanceOfClassType;
    }

    public boolean gradeC() {
        return psiVar;
    }

    public boolean gradeD() {
        return classInstCreat;
    }
}
